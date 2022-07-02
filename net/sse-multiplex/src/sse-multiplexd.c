
/*
  Copyright (c) 2015-2018, Matthias Schiffer <mschiffer@universe-factory.net>
  All rights reserved.

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions are met:

    1. Redistributions of source code must retain the above copyright notice,
       this list of conditions and the following disclaimer.
    2. Redistributions in binary form must reproduce the above copyright notice,
       this list of conditions and the following disclaimer in the documentation
       and/or other materials provided with the distribution.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
  FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include <generated/sse-multiplex.h>

#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/un.h>


typedef struct client client_t;
typedef struct provider provider_t;

typedef void (*handler_t)(provider_t *, void *buffer, size_t len);


static volatile bool running = true;

static int epoll_fd = -1;
static int listen_fd = -1;
static struct epoll_event listen_event = {};

static provider_t *providers = NULL;


typedef enum {
        CLIENT_STATE_NEW = 0,
        CLIENT_STATE_HEADER_SENT,
        CLIENT_STATE_ACTIVE,
        CLIENT_STATE_CLOSE,
} client_state_t;


struct client {
        struct client *next;

        int fd;
        client_state_t state;
};

struct provider {
        struct provider *prev;
        struct provider *next;

        char *command;
        int fd;
        struct epoll_event event;

        size_t header_buflen;
        size_t header_len;
        char *header;

        char last;
        bool clean;

        handler_t handler;
        client_t *clients;
};


static int run_command(const char *command) {
        int pipefd[2];
        if (pipe2(pipefd, O_CLOEXEC) < 0) {
                syslog(LOG_ERR, "pipe: %s", strerror(errno));
                return -1;
        }

        pid_t pid = fork();
        if (pid < 0) {
                syslog(LOG_ERR, "fork: %s", strerror(errno));
                close(pipefd[0]);
                close(pipefd[1]);
                return -1;
        }

        if (pid > 0) {
                close(pipefd[1]);
                return pipefd[0];
        }
        else {
                dup2(pipefd[1], STDOUT_FILENO);

                struct sigaction action = {};
                sigemptyset(&action.sa_mask);

                action.sa_handler = SIG_DFL;
                sigaction(SIGCHLD, &action, NULL);
                sigaction(SIGPIPE, &action, NULL);

                execl("/bin/sh", "/bin/sh", "-c", command, NULL);
                _exit(127);
        }
}

/** Write a whole buffer to a FD, retrying on partial writes */
static bool feed(int fd, void *buffer, size_t len) {
        while (len) {
                ssize_t w = write(fd, buffer, len);
                if (w == 0)
                        return false;
                if (w < 0) {
                        if (errno == EINTR)
                                continue;

                        return false;
                }

                buffer += w;
                len -= w;
        }

        return true;
}

/**
        Creates a new client for a given socket FD and adds
        it to a provider's client list
*/
static void client_add(provider_t *p, int fd) {
        client_t *c = calloc(1, sizeof(*c));
        c->fd = fd;

        c->next = p->clients;
        p->clients = c;
}

/** Writes a buffer to a client's FD if the client is active */
static void client_feed(client_t *c, void *buffer, size_t len) {
        if (!feed(c->fd, buffer, len))
                c->state = CLIENT_STATE_CLOSE;
}

static void client_free(client_t *c) {
        close(c->fd);
        free(c);
}

/** Writes a buffer to all active clients of a given provider */
static void provider_handle_data(provider_t *provider, void *buffer, size_t len) {
        if (!len)
                return;

        for (client_t *c = provider->clients; c; c = c->next) {
                if (c->state == CLIENT_STATE_ACTIVE)
                        client_feed(c, buffer, len);
        }
}

/**
        Adds a buffer to the header buffer of a provider

        The HTTP header generated by the provider is reproduced for each new
        client connecting for the same provider, so it must be stored in a
        buffer. New providers are using this hander. As soon as the input is
        clean (a double newline terminating the header has been received), this
        handler replaces itself with provider_handle_data().
*/
static void provider_handle_header(provider_t *provider, void *buffer, size_t len) {
        if (provider->clean)
                provider->handler = provider_handle_data;

        if (!len)
                return;

        size_t new_len = provider->header_len + len;
        if (new_len < provider->header_len)
                goto overflow;

        if (new_len > provider->header_buflen) {
                if (!provider->header_buflen)
                        provider->header_buflen = 128;

                while (new_len > provider->header_buflen) {
                        provider->header_buflen <<= 1;
                        if (!provider->header_buflen)
                                goto overflow;
                }

                provider->header = realloc(provider->header, provider->header_buflen);
        }

        memcpy(provider->header+provider->header_len, buffer, len);
        provider->header_len = new_len;

        return;

overflow:
        /* Just a safeguard, we're OOM long before
         * an overflow
         *
         * We can't really do anything useful here,
         * so we just drop the buffer ¯\_(ツ)_/¯
         */
        free(provider->header);
        provider->header_len = 0;
        provider->header_buflen = 0;
        return;
}

/** Runs the given command, creating a new provider for the command's stdout pipe */
static provider_t * provider_new(const char *command) {
        int fd = run_command(command);
        if (fd < 0) {
                syslog(LOG_WARNING, "unable to run command `%s'", command);
                return NULL;
        }

        fcntl(fd, F_SETFL, fcntl(fd, F_GETFL) | O_NONBLOCK);

        provider_t *p = calloc(1, sizeof(*p));
        p->command = strdup(command);
        p->fd = fd;

        p->handler = provider_handle_header;

        p->event.events = EPOLLIN|EPOLLRDHUP;
        p->event.data.ptr = p;

        if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, fd, &p->event) < 0) {
		fprintf(stderr, "epoll_ctl: %s\n", strerror(errno));
                exit(1);
        }

        if (providers)
                providers->prev = p;
        p->next = providers;
        providers = p;

        return p;
}

/**
        Either retrieves an existing provider for the given command or
        creates a new one if none exists
*/
static provider_t * provider_get(const char *command) {
        provider_t *p;
        for (p = providers; p; p = p->next) {
                if (!strcmp(p->command, command))
                        return p;
        }

        return provider_new(command);
}

/**
        Cleans up behind a provider, removes it from the global provider list
        and frees the provider */
static void provider_del(provider_t *p) {
        if (p->next)
                p->next->prev = p->prev;

        if (p->prev)
                p->prev->next = p->next;
        else
                providers = p->next;

        if (epoll_ctl(epoll_fd, EPOLL_CTL_DEL, p->fd, NULL) < 0) {
		fprintf(stderr, "epoll_ctl: %s\n", strerror(errno));
                exit(1);
        }

        for (client_t *c = p->clients; c; c = p->clients) {
                p->clients = c->next;
                client_free(c);
        }

        free(p->command);
        close(p->fd);
        free(p->header);
        free(p);
}

/**
        Periodic maintenance

        New clients are activated as soon as the input is clean (we have just
        read a double newline); clients that have failed are deleted.

        When all clients have been removed, the provider itself is deleted and
        false is returned.
*/
static bool provider_maintain(provider_t *p) {
        for (client_t **cp = &p->clients, *c = *cp; c; c = *cp) {
                switch (c->state) {
                case CLIENT_STATE_NEW:
                        if (p->handler == provider_handle_header)
                                break;

                        /*
                                Set state first, in case client_feed()
                                sets the state to CLIENT_STATE_CLOSE
                        */
                        c->state = CLIENT_STATE_HEADER_SENT;
                        client_feed(c, p->header, p->header_len);
                        continue;

                case CLIENT_STATE_HEADER_SENT:
                        if (p->clean)
                                c->state = CLIENT_STATE_ACTIVE;

                        break;

                case CLIENT_STATE_ACTIVE:
                        break;

                case CLIENT_STATE_CLOSE:
                        *cp = c->next;
                        client_free(c);
                        continue;
                }

                cp = &c->next;
        }

        if (!p->clients) {
                provider_del(p);
                return false;
        }

        return true;
}

/**
        Handles input data from a provider

        clean must be set to true if we are at the end of the header or an SSE
        record (a double newline).

        Returns false when the provider has been deleted because all clients
        disappeared.
*/
static bool provider_data(provider_t *provider, void *buf, size_t len, bool clean) {
        provider->clean = clean;
        provider->handler(provider, buf, len);
        return provider_maintain(provider);
}

static void init_epoll(void) {
        epoll_fd = epoll_create1(EPOLL_CLOEXEC);
        if (epoll_fd < 0) {
		fprintf(stderr, "Unable initialize epoll: %s\n", strerror(errno));
                exit(1);
        }
}

static void unlink_socket(void) {
        if (listen_fd >= 0) {
                unlink(SSE_MULTIPLEX_SOCKET);
                listen_fd = -1;
        }
}

static void create_socket(void) {
        listen_fd = socket(AF_UNIX, SOCK_STREAM|SOCK_CLOEXEC, 0);
        if (listen_fd < 0) {
                fprintf(stderr, "socket: %s\n", strerror(errno));
                exit(1);
        }

        size_t socket_len = strlen(SSE_MULTIPLEX_SOCKET);
	size_t len = offsetof(struct sockaddr_un, sun_path) + socket_len + 1;
	uint8_t buf[len];
	memset(buf, 0, len);

	struct sockaddr_un *sa = (void*)buf;

	sa->sun_family = AF_UNIX;
	memcpy(sa->sun_path, SSE_MULTIPLEX_SOCKET, socket_len+1);

        mode_t old_umask = umask(077);

        if (bind(listen_fd, (struct sockaddr*)sa, len)) {
		switch (errno) {
		case EADDRINUSE:
			fprintf(stderr, "Unable to bind socket: the path `%s' already exists\n", SSE_MULTIPLEX_SOCKET);
                        break;

		default:
			fprintf(stderr, "Unable to bind socket: %s\n", strerror(errno));
		}

                exit(1);
	}

        umask(old_umask);

        if (atexit(unlink_socket)) {
                fprintf(stderr, "atexit: %s", strerror(errno));
                unlink_socket();
                exit(1);
        }

        if (listen(listen_fd, 16) < 0) {
                fprintf(stderr, "listen: %s\n", strerror(errno));
                exit(1);
        }

        listen_event.events = EPOLLIN;
        listen_event.data.ptr = &listen_event;

        if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, listen_fd, &listen_event) < 0) {
		fprintf(stderr, "epoll_ctl: %s\n", strerror(errno));
                exit(1);
        }
}

static void signal_exit(int signal __attribute__((unused))) {
        running = false;
}

static void setup_signals(void) {
        struct sigaction action = {};
        sigemptyset(&action.sa_mask);

        action.sa_handler = signal_exit;
        sigaction(SIGINT, &action, NULL);
        sigaction(SIGTERM, &action, NULL);
        sigaction(SIGQUIT, &action, NULL);

        action.sa_handler = SIG_IGN;
        sigaction(SIGCHLD, &action, NULL);
        sigaction(SIGPIPE, &action, NULL);
}

/** Handles input from a provider */
static void epoll_handle_provider(provider_t *provider) {
        /*
                The "last" field contains the last character from the previously
                read buffer. This allows us to search for double newlines that
                span two reads.
        */
        struct {
                char last;
                char buf[1024];
        } data;

        data.last = provider->last;

        ssize_t r = read(provider->fd, data.buf, sizeof(data.buf));
        if (r <= 0) {
                if (r < 0 && errno == EINTR)
                        return;

                /*
                        EOF before header end: just output the whole block
                        by pretending a clean state and delete the provider
                */
                if (!provider_data(provider, NULL, 0, true))
                        return;
                provider_del(provider);
                return;
        }

        provider->last = data.buf[r-1];

        char *sep = memmem(&data, 1 + r, "\n\n", 2);

        /*
                When we found a separator, split the message, so
                provider_maintain() is run as soon as possible, in case a new
                client needs to be activated.
        */
        size_t len1 = sep ? (sep + 2) - data.buf : r;
        if (!provider_data(provider, data.buf, len1, sep))
                return;

        if (!sep)
                return;

        /*
                If there is a second chunk after the first double newline,
                we don't need to split it further: All new clients have already
                been enabled
        */
        size_t len2 = r - len1;
        if (len2)
                provider_data(
                        provider, data.buf + len1, len2,
                        data.buf[r-2] == '\n' && data.buf[r-1] == '\n'
                );
}

static void epoll_handle_accept(uint32_t events) {
        if (events != EPOLLIN) {
		syslog(LOG_ERR, "unexpected event on listening socket: %u\n", (unsigned)events);
                exit(1);
        }

        int fd = accept4(listen_fd, NULL, NULL, SOCK_CLOEXEC);
        if (fd < 0) {
		syslog(LOG_WARNING, "accept4: %s\n", strerror(errno));
                return;
        }

        char command[1024];
        char *c = command;
        while (true) {
                ssize_t r = read(fd, c, command + (sizeof(command)-1) - c);
                if (r < 0) {
                        close(fd);
                        return;
                }

                if (r == 0) {
                        *c = 0;
                        break;
                }

                c += r;
        }

        fcntl(fd, F_SETFL, fcntl(fd, F_GETFL) | O_NONBLOCK);

        provider_t *p = provider_get(command);
        if (!p) {
                close(fd);
                return;
        }

        client_add(p, fd);
        provider_maintain(p);
}

void cleanup(void) {
        while (providers)
                provider_del(providers);
}

int main() {
        init_epoll();
        create_socket();
        setup_signals();

        while (running) {
                struct epoll_event event;
                int ret = epoll_wait(epoll_fd, &event, 1, -1);
                if (ret == 0)
                        continue;

                if (ret < 0) {
                        if (errno == EINTR)
                                continue;

		        syslog(LOG_ERR, "epoll_wait: %s\n", strerror(errno));
                        exit(1);
                }

                if (event.data.ptr == &listen_event)
                        epoll_handle_accept(event.events);
                else
                        epoll_handle_provider(event.data.ptr);
        }

        cleanup();
}
