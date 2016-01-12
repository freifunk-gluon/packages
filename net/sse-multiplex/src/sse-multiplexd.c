
/*
  Copyright (c) 2015, Matthias Schiffer <mschiffer@universe-factory.net>
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


static volatile bool running = true;

static int epoll_fd = -1;
static int listen_fd = -1;
static struct epoll_event listen_event = {};

static provider_t *providers = NULL;


struct client {
        struct client *next;

        FILE *file;
        bool active;
};

struct provider {
        struct provider *prev;
        struct provider *next;

        char *command;
        FILE *file;
        struct epoll_event event;

        char *header;
        bool preclean;
        bool clean;

        client_t *clients;
};


static char * read_header(FILE *file) {
        size_t buflen = 256, content_len = 0;
        char *buffer = malloc(buflen);

        while (true) {
                size_t space = buflen - content_len;
                if (space < 128) {
                        buflen += 256;
                        buffer = realloc(buffer, buflen);
                        space = buflen - content_len;
                }

                bool ok = fgets(buffer+content_len, space, file);
                if (!ok) {
                        free(buffer);
                        return NULL;
                }

                content_len += strlen(buffer+content_len);

                if (content_len >= 2 && buffer[content_len-2] == '\n' && buffer[content_len-1] == '\n')
                        return buffer;
        }
}

static FILE * run_command(const char *command) {
        int pipefd[2];
        if (pipe(pipefd) < 0) {
                syslog(LOG_ERR, "pipe: %s", strerror(errno));
                return NULL;
        }

        pid_t pid = fork();
        if (pid < 0) {
                syslog(LOG_ERR, "fork: %s", strerror(errno));
                close(pipefd[0]);
                close(pipefd[1]);
                return NULL;
        }

        if (pid > 0) {
                close(pipefd[1]);

                FILE *file = fdopen(pipefd[0], "r");
                if (!file) {
                        close(pipefd[0]);
                        return NULL;
                }

                return file;
        }
        else {
                close(pipefd[0]);
                dup2(pipefd[1], STDOUT_FILENO);

                if (pipefd[1] != STDOUT_FILENO)
                        close(pipefd[1]);

                struct sigaction action = {};
                sigemptyset(&action.sa_mask);

                action.sa_handler = SIG_DFL;
                sigaction(SIGCHLD, &action, NULL);
                sigaction(SIGPIPE, &action, NULL);

                execl("/bin/sh", "/bin/sh", "-c", command, NULL);
                _exit(127);
        }
}

static provider_t * new_provider(const char *command) {
        FILE *file = run_command(command);
        if (!file) {
                syslog(LOG_WARNING, "unable to start provider `%s'", command);
                return NULL;
        }

        char *header = read_header(file);
        if (!header) {
                fclose(file);
                return NULL;
        }

        fcntl(fileno(file), F_SETFL, fcntl(fileno(file), F_GETFL) | O_NONBLOCK);

        provider_t *p = calloc(1, sizeof(*p));
        p->command = strdup(command);
        p->file = file;
        p->header = header;
        p->preclean = true;
        p->clean = true;

        p->event.events = EPOLLIN|EPOLLRDHUP;
        p->event.data.ptr = p;

        if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, fileno(file), &p->event) < 0) {
		fprintf(stderr, "epoll_ctl: %s\n", strerror(errno));
                exit(1);
        }

        if (providers)
                providers->prev = p;
        p->next = providers;
        providers = p;

        return p;
}

static provider_t * get_provider(const char *command) {
        provider_t *p;
        for (p = providers; p; p = p->next) {
                if (!strcmp(p->command, command))
                        return p;
        }

        return new_provider(command);
}

static void free_clients(client_t *clients) {
        while (clients) {
                client_t *next = clients->next;

                fclose(clients->file);
                free(clients);

                clients = next;
        }
}

static void free_provider(provider_t *p) {
        if (epoll_ctl(epoll_fd, EPOLL_CTL_DEL, fileno(p->file), NULL) < 0) {
		fprintf(stderr, "epoll_ctl: %s\n", strerror(errno));
                exit(1);
        }

        free(p->command);
        fclose(p->file);
        free(p->header);
        free_clients(p->clients);
        free(p);
}

static void remove_provider(provider_t *p) {
        if (p->next)
                p->next->prev = p->prev;

        if (p->prev)
                p->prev->next = p->next;
        else
                providers = p->next;

        free_provider(p);
}

static void add_client(provider_t *p, FILE *file) {
        if (fputs(p->header, file) == EOF || fflush(file) == EOF || ferror(file)) {
                fclose(file);
                return;
        }

        client_t *c = calloc(1, sizeof(*c));
        c->file = file;
        c->active = p->clean;

        c->next = p->clients;
        p->clients = c;
}

static void remove_client(client_t **client) {
        client_t *c = *client;
        *client = c->next;

        fclose(c->file);
        free(c);
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

static void handle_data(provider_t *provider) {
        while (true) {
                if (feof(provider->file)) {
                        remove_provider(provider);
                        return;
                }

                char buf[1024];
                bool ok = fgets(buf, sizeof(buf), provider->file);
                if (!ok)
                        return;

                provider->clean = provider->preclean && (buf[0] == '\n');
                provider->preclean = (buf[strlen(buf)-1] == '\n');

                client_t **c = &provider->clients;
                while (*c) {
                        if ((*c)->active && fputs(buf, (*c)->file) == EOF) {
                                remove_client(c);
                                continue;
                        }

                        if (provider->clean) {
                                /* The ferror check should be redundant, as flush
                                 * should already return EOF on errors; on uClibc,
                                 * it sometimes doesn't... */
                                if (fflush((*c)->file) == EOF || ferror((*c)->file)) {
                                        remove_client(c);
                                        continue;
                                }

                                (*c)->active = true;
                        }

                        c = &(*c)->next;
                }

                if (!provider->clients) {
                        remove_provider(provider);
                        return;
                }
        }
}

static void handle_accept(uint32_t events) {
        if (events != EPOLLIN) {
		syslog(LOG_ERR, "unexpected event on listening socket: %u\n", (unsigned)events);
                exit(1);
        }

        int fd = accept4(listen_fd, NULL, NULL, SOCK_CLOEXEC);
        if (fd < 0) {
		syslog(LOG_WARNING, "accept4: %s\n", strerror(errno));
                return;
        }

        FILE *file = fdopen(fd, "r+");
        if (!file) {
		syslog(LOG_WARNING, "fdopen: %s\n", strerror(errno));
                close(fd);
                return;
        }

        char command[1024];
        bool ok = fgets(command, sizeof(command), file);
        if (!ok || !command[0] || !feof(file)) {
                fclose(file);
                return;
        }

        fcntl(fd, F_SETFL, fcntl(fd, F_GETFL) | O_NONBLOCK);

        provider_t *p = get_provider(command);
        if (!p) {
                fclose(file);
                return;
        }

        add_client(p, file);
        handle_data(p);
}

void cleanup(void) {
        while (providers)
                remove_provider(providers);
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
                        handle_accept(event.events);
                else
                        handle_data(event.data.ptr);
        }

        cleanup();
}
