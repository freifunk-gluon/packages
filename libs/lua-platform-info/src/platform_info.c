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


#include <libplatforminfo.h>

#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>


static int push_string(lua_State *L, const char *value) {
	if (value)
		lua_pushstring(L, value);
	else
		lua_pushnil(L);

	return 1;
}


static int get_target(lua_State *L) {
	return push_string(L, platforminfo_get_target());
}

static int get_subtarget(lua_State *L) {
	return push_string(L, platforminfo_get_subtarget());
}

static int get_board_name(lua_State *L) {
	return push_string(L, platforminfo_get_board_name());
}

static int get_model(lua_State *L) {
	return push_string(L, platforminfo_get_model());
}

static int get_image_name(lua_State *L) {
	return push_string(L, platforminfo_get_image_name());
}


static const luaL_reg R[] = {
	{"get_target", get_target},
	{"get_subtarget", get_subtarget},
	{"get_board_name", get_board_name},
	{"get_model", get_model},
	{"get_image_name", get_image_name},
	{NULL, NULL },
};

LUALIB_API int luaopen_platform_info(lua_State *L) {
	luaL_register(L, "platform_info", R);
	return 1;
}
