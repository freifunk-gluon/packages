/* SPDX-FileCopyrightText: 2021-2023 Maciej Krüger <maciej@xeredo.it> */
/* SPDX-License-Identifier: BSD-2-Clause */

#include "libolsrdhelper.h"
#include "uclient.h"

#include <errno.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/wait.h>
#include <libubox/uloop.h>

/* jsoninfo ports */
#define BASE_URL_4 "http://127.0.0.1:9090"
#define BASE_URL_6 "http://[::1]:9091"

#define READ_CHUNK (64 * 1024)

/** Response buffer filled by the uclient callbacks */
struct recv_ctx {
	char *data;
	size_t size;
	size_t capacity;
	int error;

	json_object *parsed;
};

/** Reads everything uclient has buffered into ctx->data */
static void recv_cb(struct uclient *cl) {
	struct recv_ctx *ctx = uclient_get_custom(cl);

	while (true) {
		if (ctx->size + READ_CHUNK + 1 > ctx->capacity) {
			size_t capacity = ctx->capacity ? ctx->capacity * 2 : 2 * READ_CHUNK;
			char *data = realloc(ctx->data, capacity);

			if (!data) {
				ctx->error = UCLIENT_ERROR_OUT_OF_MEMORY;
				return;
			}

			ctx->data = data;
			ctx->capacity = capacity;
		}

		ssize_t len = uclient_read_account(cl, ctx->data + ctx->size, ctx->capacity - ctx->size - 1);

		if (len < 0) {
			ctx->error = UCLIENT_ERROR_CONNECTION_RESET_PREMATURELY;
			return;
		}

		if (!len)
			return;

		ctx->size += len;
	}
}

static void recv_eof_cb(struct uclient *cl) {
	struct recv_ctx *ctx = uclient_get_custom(cl);

	if (ctx->error)
		return;

	if (!ctx->size || ctx->data[0] != '{') {
		ctx->error = UCLIENT_ERROR_NOT_JSON;
		return;
	}

	ctx->data[ctx->size] = '\0';

	ctx->parsed = json_tokener_parse(ctx->data);
	if (!ctx->parsed)
		ctx->error = UCLIENT_ERROR_NOT_JSON;
}

static const char * olsr_base_url(int ipv) {
	switch (ipv) {
	case OLSR_IPV4:
		return BASE_URL_4;
	case OLSR_IPV6:
		return BASE_URL_6;
	default:
		return NULL;
	}
}

/** execv cmd with the NULL terminated args, true if it exited 0 */
static bool success_exit(const char *cmd, ...) {
	pid_t pid = fork();

	if (pid < 0)
		return false;

	if (!pid) {
		va_list val;
		int argc = 2; // leading command + trailing NULL

		va_start(val, cmd);
		while (va_arg(val, char *) != NULL)
			argc++;
		va_end(val);

		char **args = malloc(argc * sizeof(char *));
		if (!args)
			_exit(1);

		args[0] = (char *)cmd;

		va_start(val, cmd);
		int i = 0;
		do {
			i++;
			args[i] = va_arg(val, char *);
			// copies the trailing NULL too
		} while (args[i] != NULL);
		va_end(val);

		execv(cmd, args);
		_exit(1);
	}

	int status;

	if (waitpid(pid, &status, 0) == -1)
		return false;

	return WIFEXITED(status) && WEXITSTATUS(status) == 0;
}

int olsr_get_info(json_object *site, struct olsr_info *out) {
	if (!site)
		return 1;

	*out = (struct olsr_info){
		.olsr4 = {
			.enabled = false,
			.running = false,
		},
		.olsr6 = {
			.enabled = false,
			.running = false,
		}
	};

	if (json_object_object_get(site, "prefix4")) {
		out->olsr4.enabled = true;
		out->olsr4.running = success_exit("/etc/init.d/olsrd", "running", NULL);
	}

	if (json_object_object_get(site, "prefix6")) {
		out->olsr6.enabled = true;
		out->olsr6.running = success_exit("/etc/init.d/olsrd6", "running", NULL);
	}

	return 0;
}

int olsr_get_nodeinfo(int ipv, const char *path, json_object **out) {
	const char *base_url = olsr_base_url(ipv);
	if (!base_url)
		return UCLIENT_ERROR_CONNECT;

	char url[strlen(base_url) + strlen(path) + 2];
	sprintf(url, "%s/%s", base_url, path);

	struct recv_ctx ctx = { };

	uloop_init();
	int err_code = get_url(url, &recv_cb, &recv_eof_cb, &ctx, -1);
	uloop_done();

	free(ctx.data);

	if (!err_code)
		err_code = ctx.error;

	if (err_code) {
		json_object_put(ctx.parsed);
		return err_code;
	}

	*out = ctx.parsed;

	return 0;
}

/* out is optional, without it the raw fd is returned */
int socket_request(const char *path, const char *cmd, char **out) {
	int fd = socket(AF_UNIX, SOCK_STREAM, 0);
	if (fd < 0)
		return -errno;

	struct sockaddr_un addr = {};
	addr.sun_family = AF_UNIX;

	if (strlen(path) >= sizeof(addr.sun_path)) {
		errno = ENAMETOOLONG;
		goto err;
	}

	strcpy(addr.sun_path, path);

	if (connect(fd, (struct sockaddr *)&addr, sizeof(addr)) == -1)
		goto err;

	if (send(fd, cmd, strlen(cmd), 0) == -1)
		goto err;

	if (shutdown(fd, SHUT_WR))
		goto err;

	if (!out)
		return fd;

	size_t size = 0, capacity = READ_CHUNK;
	char *data = malloc(capacity);

	if (!data) {
		errno = ENOMEM;
		goto err;
	}

	while (true) {
		if (size + 1 == capacity) {
			char *bigger = realloc(data, capacity * 2);

			if (!bigger) {
				free(data);
				errno = ENOMEM;
				goto err;
			}

			data = bigger;
			capacity *= 2;
		}

		ssize_t len = recv(fd, data + size, capacity - size - 1, 0);

		if (len < 0) {
			free(data);
			goto err;
		}

		if (!len)
			break;

		size += len;
	}

	data[size] = '\0';
	*out = data;

	close(fd);

	return 0;

err: ;
	int err = errno;
	close(fd);

	return -err;
}

json_object * socket_request_json(const char *path, const char *cmd) {
	int fd = socket_request(path, cmd, NULL);
	if (fd < 0)
		return NULL;

	json_object *ret = json_object_from_fd(fd);

	close(fd);

	return ret;
}
