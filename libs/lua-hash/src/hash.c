// SPDX-License-Identifier: MIT License
// SPDX-FileCopyrightText: 2016-2021 Leonardo Mörlein <git@irrelefant.net>
#include <libubox/md5.h>

#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
#include <stdlib.h>
#include <string.h>

#define MD5_DIGEST_LEN 16
#define MD5_STRLEN (2*MD5_DIGEST_LEN+1)

static int md5(lua_State *L) {
	size_t len;
	const char *input = lua_tolstring(L, 1, &len);
	if (input == NULL) {
		lua_pushstring(L, "first argument: should be the input string");
		lua_error(L);
		__builtin_unreachable();
	}

	unsigned char digest[MD5_DIGEST_LEN];
	char output[MD5_STRLEN];

	struct md5_ctx ctx;
	md5_begin(&ctx);
	md5_hash(input, len, &ctx);
	md5_end(digest, &ctx);

	// fill the digest bytewise in the output string
	int i;
	for (i = 0; i < MD5_DIGEST_LEN; i++) {
		sprintf(output + 2*i, "%02x", digest[i]);
	}

	lua_pushstring(L, output);
	return 1;
}

static const luaL_Reg R[] = {
	{"md5", md5},
	{NULL, NULL },
};

LUALIB_API int luaopen_hash(lua_State *L) {
	luaL_register(L, "hash", R);
	return 1;
}
