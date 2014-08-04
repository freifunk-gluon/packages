
/*
  Copyright (c) 2014, Matthias Schiffer <mschiffer@universe-factory.net>
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


#define _GNU_SOURCE

#include <errno.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/socket.h>

#include <net/if.h>

#include <linux/ethtool.h>
#include <linux/sockios.h>

#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>


struct stats_context {
	struct ifreq ifr;
	int fd;

	struct ethtool_gstrings *strings;
	struct ethtool_stats *stats;
};


static inline void do_ioctl(lua_State *L, struct stats_context *ctx, void *data) {
	ctx->ifr.ifr_data = data;
	if (ioctl(ctx->fd, SIOCETHTOOL, &ctx->ifr) < 0)
		luaL_error(L, "ioctl: %s", strerror(errno));
}

static inline uint32_t get_stats_length(lua_State *L, struct stats_context *ctx) {
	const size_t sset_info_len = sizeof(struct ethtool_sset_info) + sizeof(uint32_t);
	struct ethtool_sset_info *sset_info = alloca(sset_info_len);
	memset(sset_info, 0, sset_info_len);

	sset_info->cmd = ETHTOOL_GSSET_INFO;
	sset_info->sset_mask = 1ull << ETH_SS_STATS;
	do_ioctl(L, ctx, sset_info);

	return sset_info->sset_mask ? sset_info->data[0] : 0;
}

static inline void get_stats_strings(lua_State *L, struct stats_context *ctx) {
	uint32_t n_stats = get_stats_length(L, ctx);

	if (!n_stats)
		return;

	ctx->strings = calloc(1, sizeof(*ctx->strings) + n_stats * ETH_GSTRING_LEN);
	if (!ctx->strings) {
		luaL_error(L, "calloc: %s", strerror(errno));
		return;
	}

	ctx->strings->cmd = ETHTOOL_GSTRINGS;
	ctx->strings->string_set = ETH_SS_STATS;
	ctx->strings->len = n_stats;

	do_ioctl(L, ctx, ctx->strings);
}

static inline int get_stats(lua_State *L, struct stats_context *ctx) {
	get_stats_strings(L, ctx);

	if (!ctx->strings) {
		lua_newtable(L);
		return 1;
	}

	ctx->stats = calloc(1, sizeof(struct ethtool_stats) + ctx->strings->len * sizeof(uint64_t));
	if (!ctx->stats)
		return luaL_error(L, "calloc: %s", strerror(errno));

	ctx->stats->cmd = ETHTOOL_GSTATS;
	ctx->stats->n_stats = ctx->strings->len;

	do_ioctl(L, ctx, ctx->stats);

	lua_createtable(L, 0, ctx->strings->len);

	size_t i;
	for (i = 0; i < ctx->strings->len; i++) {
		const char *key = (const char*)&ctx->strings->data[i * ETH_GSTRING_LEN];
		lua_pushlstring(L, key, strnlen(key, ETH_GSTRING_LEN));
		lua_pushnumber(L, (lua_Number)ctx->stats->data[i]);
		lua_settable(L, -3);
	}

	return 1;
}


static int interface_stats(lua_State *L) {
	const char *ifname = luaL_checkstring(L, 1);

	struct stats_context *ctx = lua_newuserdata(L, sizeof(*ctx));
	memset(ctx, 0, sizeof(*ctx));

	luaL_getmetatable(L, "ethtool_stats.ctx");
	lua_setmetatable(L, -2);

	strncpy(ctx->ifr.ifr_name, ifname, IFNAMSIZ);

	ctx->fd = socket(AF_INET, SOCK_DGRAM, 0);
	if (ctx->fd < 0)
		return luaL_error(L, "socket: %s", strerror(errno));

	return get_stats(L, ctx);
}

static int ctx_gc(lua_State *L) {
	struct stats_context *ctx = lua_touserdata(L, 1);

	if (ctx->fd >= 0)
		close(ctx->fd);

	free(ctx->strings);
	free(ctx->stats);

	return 0;
}

static const luaL_reg R[] = {
	{"interface_stats", interface_stats},
	{NULL, NULL },
};

LUALIB_API int luaopen_ethtool_stats(lua_State *L) {
	luaL_newmetatable(L, "ethtool_stats.ctx");
	lua_pushstring(L, "__gc");
	lua_pushcfunction(L, ctx_gc);
	lua_settable(L, -3);

	luaL_register(L, "ethtool_stats", R);
	return 1;
}
