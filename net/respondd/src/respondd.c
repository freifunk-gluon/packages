/*
   Copyright (c) 2014-2015, Nils Schneider <nils@nilsschneider.net>
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

#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <arpa/inet.h>
#include <net/if.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>


static void usage() {
  puts("Usage: respondd [-h] -g <group> -p <port> -i <if0> [-i <if1> ..]");
  puts("  -g <ip6>         multicast group, e.g. ff02::2:1001");
  puts("  -p <int>         port number to listen on");
  puts("  -i <string>      interface on which the group is joined");
  puts("  -c <string>      Lua command");
  puts("  -h               this help\n");
}

static void join_mcast(const int sock, const struct in6_addr addr, const char *iface) {
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
}

static void serve(lua_State *L, int sock) {
  char buffer[256];
  ssize_t read_bytes;

  lua_pushvalue(L, -1);

  struct sockaddr_in6 addr;
  socklen_t addrlen = sizeof(addr);

  read_bytes = recvfrom(sock, buffer, sizeof(buffer), 0, (struct sockaddr *)&addr, &addrlen);

  if (read_bytes < 0) {
    perror("recvfrom failed");
    exit(EXIT_FAILURE);
  }

  lua_pushlstring(L, buffer, read_bytes);

  if (lua_pcall(L, 1, 1, 0)) {
    fprintf(stderr, "%s\n", lua_tostring(L, -1));
  }
  else if (lua_isstring(L, -1)) {
    size_t msg_length;
    const char *msg = lua_tolstring(L, -1, &msg_length);

    if (sendto(sock, msg, msg_length, 0, (struct sockaddr *)&addr, addrlen) < 0)
      perror("sendto failed");
  }

  lua_pop(L, 1);
}

int main(int argc, char **argv) {
  int sock;
  struct sockaddr_in6 server_addr = {};
  struct in6_addr mgroup_addr;
  const char *command = NULL;

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
  while ((c = getopt(argc, argv, "p:g:i:c:h")) != -1) {
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

    case 'c':
      command = optarg;
      break;

    case 'h':
      usage();
      exit(EXIT_SUCCESS);
      break;

    default:
      fprintf(stderr, "Invalid parameter %c ignored.\n", c);
    }
  }

  if (!command) {
    fprintf(stderr, "No command given.\n");
    exit(EXIT_FAILURE);
  }

  if (bind(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
    perror("bind failed");
    exit(EXIT_FAILURE);
  }

  lua_State *L = luaL_newstate();
  luaL_openlibs(L);

  if (luaL_loadstring(L, command)) {
    perror("Unable to load Lua command");
    exit(EXIT_FAILURE);
  }

  lua_call(L, 0, 1);

  while (1)
    serve(L, sock);

  return EXIT_FAILURE;
}
