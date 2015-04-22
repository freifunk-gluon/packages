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

#include "miniz.c"

#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

#include <stdlib.h>


static int l_compress(lua_State *L) {
	size_t in_length;
	const unsigned char *in = (unsigned char *)luaL_checklstring(L, 1, &in_length);

	mz_ulong out_length = mz_compressBound(in_length);
	unsigned char *out = malloc(out_length);

	int status = mz_compress(out, &out_length, in, in_length);
	if (status) {
		free(out);

		lua_pushnil(L);
		lua_pushstring(L, mz_error(status));
		return 2;
	}

	lua_pushlstring(L, (const char*)out, out_length);
	free(out);

	return 1;
}

static const luaL_reg R[] = {
	{"compress", l_compress},
	{NULL, NULL },
};

LUALIB_API int luaopen_deflate(lua_State *L) {
	luaL_register(L, "deflate", R);
	return 1;
}
