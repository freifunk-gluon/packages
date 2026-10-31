// SPDX-License-Identifier: BSD-2-Clause
// SPDX-FileCopyrightText: 2017 Jan-Philipp Litza <janphilipp@litza.de>
#pragma once


#include <libubox/uclient.h>
#include <sys/types.h>


enum uclient_own_error_code {
	UCLIENT_ERROR_REDIRECT_FAILED = 32,
	UCLIENT_ERROR_TOO_MANY_REDIRECTS,
	UCLIENT_ERROR_CONNECTION_RESET_PREMATURELY,
	UCLIENT_ERROR_SIZE_MISMATCH,
	UCLIENT_ERROR_STATUS_CODE = 1024,
	UCLIENT_ERROR_INTERRUPTED = 2048,
};


struct uclient_data {
	/* data that can be passed in by caller and used in custom callbacks */
	void *custom;
	/* data used by uclient callbacks */
	int retries;
	int err_code;
	ssize_t downloaded;
	ssize_t length;
};

inline struct uclient_data * uclient_data(struct uclient *cl) {
	return (struct uclient_data *)cl->priv;
}

inline void * uclient_get_custom(struct uclient *cl) {
	return uclient_data(cl)->custom;
}


ssize_t uclient_read_account(struct uclient *cl, char *buf, int len);

int get_url(const char *url, void (*read_cb)(struct uclient *cl), void *cb_data, ssize_t len, const char *firmware_version);
const char *uclient_get_errmsg(int code);
int uclient_interrupted_signal(int code);
