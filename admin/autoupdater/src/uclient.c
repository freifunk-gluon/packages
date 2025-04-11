// SPDX-License-Identifier: BSD-2-Clause
// SPDX-FileCopyrightText: 2017 Jan-Philipp Litza <janphilipp@litza.de>


#include "uclient.h"

#include <libubox/blobmsg.h>
#include <libubox/uloop.h>

#include <limits.h>
#include <stdio.h>


#define TIMEOUT_MSEC 300000

static const char *const user_agent = "Gluon Autoupdater (using libuclient)";

enum uclient_own_error_code {
	UCLIENT_ERROR_REDIRECT_FAILED = 32,
	UCLIENT_ERROR_TOO_MANY_REDIRECTS,
	UCLIENT_ERROR_CONNECTION_RESET_PREMATURELY,
	UCLIENT_ERROR_SIZE_MISMATCH,
	UCLIENT_ERROR_STATUS_CODE = 1024,
	UCLIENT_ERROR_INTERRUPTED = 2048,
};


const char *uclient_get_errmsg(int code) {
	static char http_code_errmsg[34];
	if (code & UCLIENT_ERROR_STATUS_CODE) {
		snprintf(http_code_errmsg, sizeof(http_code_errmsg),
			"HTTP error %d", code & (~UCLIENT_ERROR_STATUS_CODE));
		return http_code_errmsg;
	}
	if (code & UCLIENT_ERROR_INTERRUPTED) {
		snprintf(http_code_errmsg, sizeof(http_code_errmsg),
			"Interrupted by signal %d",
			code & (~UCLIENT_ERROR_INTERRUPTED));
		return http_code_errmsg;
	}
	switch(code) {
	case UCLIENT_ERROR_CONNECT:
		return "Connection failed";
	case UCLIENT_ERROR_TIMEDOUT:
		return "Connection timed out";
	case UCLIENT_ERROR_REDIRECT_FAILED:
		return "Failed to redirect";
	case UCLIENT_ERROR_TOO_MANY_REDIRECTS:
		return "Too many redirects";
	case UCLIENT_ERROR_CONNECTION_RESET_PREMATURELY:
		return "Connection reset prematurely";
	case UCLIENT_ERROR_SIZE_MISMATCH:
		return "Incorrect file size";
	default:
		return "Unknown error";
	}
}

int uclient_interrupted_signal(int code) {
	if (code & UCLIENT_ERROR_INTERRUPTED)
		return code & (~UCLIENT_ERROR_INTERRUPTED);

	return 0;
}


static void request_done(struct uclient *cl, int err_code) {
	uclient_data(cl)->err_code = err_code;
	uloop_end();
}


static void header_done_cb(struct uclient *cl) {
	const struct blobmsg_policy policy = {
		.name = "content-length",
		.type = BLOBMSG_TYPE_STRING,
	};
	struct blob_attr *tb_len;

	if (uclient_data(cl)->retries < 10) {
		int ret = uclient_http_redirect(cl);
		if (ret < 0) {
			request_done(cl, UCLIENT_ERROR_REDIRECT_FAILED);
			return;
		}
		if (ret > 0) {
			uclient_data(cl)->retries++;
			return;
		}
	}

	switch (cl->status_code) {
	case 200:
		break;
	case 301:
	case 302:
	case 307:
		request_done(cl, UCLIENT_ERROR_TOO_MANY_REDIRECTS);
		return;
	default:
		request_done(cl, UCLIENT_ERROR_STATUS_CODE | cl->status_code);
		return;
	}

	blobmsg_parse(&policy, 1, &tb_len, blob_data(cl->meta), blob_len(cl->meta));
	if (tb_len) {
		char *endptr;

		errno = 0;
		unsigned long long val = strtoull(blobmsg_get_string(tb_len), &endptr, 10);
		if (!errno && !*endptr && val <= SSIZE_MAX) {
			if (uclient_data(cl)->length >= 0 && uclient_data(cl)->length != (ssize_t)val) {
				request_done(cl, UCLIENT_ERROR_SIZE_MISMATCH);
				return;
			}

			uclient_data(cl)->length = val;
		}
	}
}


static void eof_cb(struct uclient *cl) {
	request_done(cl, cl->data_eof ? 0 : UCLIENT_ERROR_CONNECTION_RESET_PREMATURELY);
}


ssize_t uclient_read_account(struct uclient *cl, char *buf, int len) {
	struct uclient_data *d = uclient_data(cl);
	int r = uclient_read(cl, buf, len);

	if (r >= 0) {
		d->downloaded += r;

		if (d->length >= 0 && d->downloaded > d->length) {
			request_done(cl, UCLIENT_ERROR_SIZE_MISMATCH);
			return -1;
		}
	}

	return r;
}


int get_url(const char *url, void (*read_cb)(struct uclient *cl), void *cb_data, ssize_t len, const char *firmware_version) {
	struct uclient_data d = { .custom = cb_data, .length = len };
	struct uclient_cb cb = {
		.header_done = header_done_cb,
		.data_read = read_cb,
		.data_eof = eof_cb,
		.error = request_done,
	};
	int ret = UCLIENT_ERROR_CONNECT;

	struct uclient *cl = uclient_new(url, NULL, &cb);
	if (!cl)
		goto err;

	cl->priv = &d;
	if (uclient_set_timeout(cl, TIMEOUT_MSEC))
		goto err;
	if (uclient_connect(cl))
		goto err;
	if (uclient_http_set_request_type(cl, "GET"))
		goto err;
	if (uclient_http_reset_headers(cl))
		goto err;
	if (uclient_http_set_header(cl, "User-Agent", user_agent))
		goto err;
	if (firmware_version != NULL) {
		if (uclient_http_set_header(cl, "X-Firmware-Version", firmware_version))
			goto err;
	}
	if (uclient_request(cl))
		goto err;

	ret = uloop_run();
	if (ret) {
		/* uloop_run() returns a signal number when interrupted */
		ret |= UCLIENT_ERROR_INTERRUPTED;
		goto err;
	}

	if (!d.err_code && d.length >= 0 && d.downloaded != d.length) {
		ret = UCLIENT_ERROR_SIZE_MISMATCH;
		goto err;
	}

	ret = d.err_code;

err:
	if (cl) {
		uclient_disconnect(cl);
		uclient_free(cl);
	}

	return ret;
}
