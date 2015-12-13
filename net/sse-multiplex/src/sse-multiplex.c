
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
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <sys/socket.h>
#include <sys/un.h>


int main(int argc, char *argv[]) {
	if (argc != 2) {
		fprintf(stderr, "Usage: %s <command>\n", argv[0]);
		return 1;
	}

        size_t socket_len = strlen(SSE_MULTIPLEX_SOCKET);
	size_t len = offsetof(struct sockaddr_un, sun_path) + socket_len + 1;
	uint8_t addrbuf[len];
	memset(addrbuf, 0, len);

	struct sockaddr_un *sa = (void*)addrbuf;

	sa->sun_family = AF_UNIX;
	memcpy(sa->sun_path, SSE_MULTIPLEX_SOCKET, socket_len+1);

	int fd = socket(AF_UNIX, SOCK_STREAM, 0);
	if (fd < 0) {
		fprintf(stderr, "Failed to create socket: %s\n", strerror(errno));
		return 1;
	}

	if (connect(fd, (struct sockaddr*)sa, sizeof(addrbuf)) < 0) {
		fprintf(stderr, "Can't connect to `%s': %s\n", SSE_MULTIPLEX_SOCKET, strerror(errno));
		return 1;
	}

	char *command = argv[1];
	while (command[0]) {
		ssize_t w = write(fd, command, strlen(command));
		if (w < 0) {
			fprintf(stderr, "Can't write command: %s\n", strerror(errno));
			return 1;
		}

		command += w;
	}

	if (shutdown(fd, SHUT_WR) < 0) {
		fprintf(stderr, "shutdown: %s\n", strerror(errno));
		return 1;
	}

	setlinebuf(stdout);

	char buf[1024];
	ssize_t r;
	while (1) {
		r = recv(fd, buf, sizeof(buf), 0);
		if (r < 0) {
			fprintf(stderr, "read: %s\n", strerror(errno));
			return 1;
		}

		if (r == 0)
			return 0;

		fwrite(buf, r, 1, stdout);
	}
}
