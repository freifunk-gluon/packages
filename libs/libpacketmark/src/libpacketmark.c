// SPDX-License-Identifier: BSD-2-Clause
// SPDX-FileCopyrightText: 2014 Matthias Schiffer <mschiffer@universe-factory.net>


#define _GNU_SOURCE

#include <dlfcn.h>
#include <errno.h>
#include <stdlib.h>

#include <sys/types.h>
#include <sys/socket.h>


static int mark;
static int (*socket_real)(int domain, int type, int protocol);


__attribute__((constructor))
static void init(void) {
	const char *str = getenv("LIBPACKETMARK_MARK");
	if (str)
		mark = atoi(str);

	socket_real = dlsym(RTLD_NEXT, "socket");
}


int socket(int domain, int type, int protocol) {
	int fd = socket_real(domain, type, protocol);

	if (fd >= 0) {
		int errno_save = errno;
		setsockopt(fd, SOL_SOCKET, SO_MARK, &mark, sizeof(mark));
		errno = errno_save;
	}

	return fd;
}
