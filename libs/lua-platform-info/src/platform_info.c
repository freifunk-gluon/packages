// SPDX-License-Identifier: BSD-2-Clause
// SPDX-FileCopyrightText: 2015 Matthias Schiffer <mschiffer@universe-factory.net>


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
