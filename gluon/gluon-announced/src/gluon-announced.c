/*
   Copyright (c) 2014, Nils Schneider <nils@nilsschneider.net>
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

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <net/if.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
#include "miniz.c"

#define HELPER "/lib/gluon/announced/helper.lua"

void usage() {
  puts("Usage: gluon-announced [-h] -g <group> -p <port> -i <if0> [-i <if1> ..]");
  puts("  -g <ip6>         multicast group, e.g. ff02:0:0:0:0:0:2:1001");
  puts("  -p <int>         port number to listen on");
  puts("  -i <string>      interface on which the group is joined");
  puts("  -h               this help\n");
}

int l_deflate(lua_State *L) {
  size_t in_length, out_length;
  char *in, *out;

  in = luaL_checklstring(L, -1, &in_length);

  out_length = in_length * 2;
  out = malloc(out_length);

  compress(out, &out_length, in, in_length);

  lua_pushlstring(L, out, out_length);
  free(out);

  return 1;
}

void join_mcast(const int sock, const struct in6_addr addr, const char *iface) {
  struct ipv6_mreq mreq;

  mreq.ipv6mr_multiaddr = addr;
  mreq.ipv6mr_interface = if_nametoindex(iface);

  if (mreq.ipv6mr_interface == 0)
    goto error;

  if (setsockopt(sock, IPPROTO_IPV6, IPV6_JOIN_GROUP, &mreq, sizeof(mreq)) == -1)
    goto error;

  return;

error:
  fprintf(stderr, "Could not join multicast group on %s: ", iface);
  perror(NULL);
  return;
}

#define REQUESTSIZE 256

char *recvrequest(const int sock, struct sockaddr *client_addr, socklen_t *clilen) {
  char request_buffer[REQUESTSIZE];
  ssize_t read_bytes;

  read_bytes = recvfrom(sock, request_buffer, sizeof(request_buffer), 0, client_addr, clilen);

  if (read_bytes < 0) {
    perror("recvfrom failed");
    exit(EXIT_FAILURE);
  }

  char *request = strndup(request_buffer, read_bytes);

  if (request == NULL)
    perror("Could not receive request");

  return request;
}

void serve(const int sock, lua_State *L) {
  char *request;
  socklen_t clilen;
  struct sockaddr_in6 client_addr;

  clilen = sizeof(client_addr);

  while (1) {
    request = recvrequest(sock, (struct sockaddr*)&client_addr, &clilen);

    lua_getglobal(L, "request");
    lua_pushstring(L, request);
    free(request);

    if (lua_pcall(L, 1, 1, 0)) {
      perror("pcall on request failed");
      exit(EXIT_FAILURE);
    }

    const char *msg;
    size_t msg_length;
    msg = lua_tolstring(L, -1, &msg_length);

    if (msg == NULL) {
      lua_pop(L, -1);
      continue;
    }

    if (sendto(sock, msg, msg_length, 0, (struct sockaddr *)&client_addr, sizeof(client_addr)) < 0) {
      perror("sendto failed");
      exit(EXIT_FAILURE);
    }

    lua_pop(L, -1);
  }
}

int main(int argc, char **argv) {
  int sock;
  struct sockaddr_in6 server_addr = {};
  struct in6_addr mgroup_addr;

  sock = socket(PF_INET6, SOCK_DGRAM, 0);

  if (sock < 0) {
    perror("creating socket");
    exit(EXIT_FAILURE);
  }

  server_addr.sin6_family = AF_INET6;
  server_addr.sin6_addr = in6addr_any;

  opterr = 0;

  int group_set = 0;

  int c;
  while ((c = getopt(argc, argv, "p:g:i:h")) != -1)
    switch (c) {
      case 'p':
        server_addr.sin6_port = htons(atoi(optarg));
        break;
      case 'g':
        if (!inet_pton(AF_INET6, optarg, &mgroup_addr)) {
          perror("Invalid multicast group. This message will probably confuse you");
          exit(EXIT_FAILURE);
        }

        group_set = 1;
        break;
      case 'i':
        if (!group_set) {
          fprintf(stderr, "Multicast group must be given before interface.\n");
          exit(EXIT_FAILURE);
        }
        join_mcast(sock, mgroup_addr, optarg);
        break;
      case 'h':
        usage();
        exit(EXIT_SUCCESS);
        break;
      default:
        fprintf(stderr, "Invalid parameter %c ignored.\n", c);
    }

  if (bind(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
    perror("bind failed");
    exit(EXIT_FAILURE);
  }

  lua_State *L = lua_open();
  luaL_openlibs(L);

  lua_pushcfunction(L, l_deflate);
  lua_setglobal(L, "deflate");

  if (luaL_loadfile(L, HELPER)) {
    perror("Could not load helper.lua");
    exit(EXIT_FAILURE);
  }

  if (lua_pcall(L, 0, 0, 0)) {
    perror("pcall failed");
    exit(EXIT_FAILURE);
  }

  serve(sock, L);

  return EXIT_FAILURE;
}
