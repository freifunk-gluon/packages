/*
Copyright 2015 Jo-Philipp Wich <jow@openwrt.org>
Copyright 2018 Matthias Schiffer <mschiffer@universe-factory.net>

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

	http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/

#include "lua-jsonc.h"

#include <limits.h>
#include <lualib.h>
#include <lauxlib.h>


void lua_jsonc_push_json(lua_State *L, struct json_object *obj) {
	int64_t i;

	switch (json_object_get_type(obj)) {
	case json_type_object:
		lua_newtable(L);
		json_object_object_foreach(obj, key, val) {
			lua_jsonc_push_json(L, val);
			lua_setfield(L, -2, key);
		}
		break;

	case json_type_array:
		lua_newtable(L);
		for (size_t n = 0; n < json_object_array_length(obj); n++) {
			lua_jsonc_push_json(L, json_object_array_get_idx(obj, n));
			lua_rawseti(L, -2, n + 1);
		}
		break;

	case json_type_boolean:
		lua_pushboolean(L, json_object_get_boolean(obj));
		break;

	case json_type_int:
		i = json_object_get_int64(obj);
		if (i == (int64_t)(lua_Integer)i)
			lua_pushinteger(L, i);
		else
			lua_pushnumber(L, i);
		break;

	case json_type_double:
		lua_pushnumber(L, json_object_get_double(obj));
		break;

	case json_type_string:
		lua_pushstring(L, json_object_get_string(obj));
		break;

	case json_type_null:
		lua_pushnil(L);
		break;
	}
}

static int lua_jsonc_lua_test_array(lua_State *L, int index) {
	int max = 0;

	lua_pushnil(L);

	/* check for non-integer keys */
	while (lua_next(L, index)) {
		if (lua_type(L, -2) != LUA_TNUMBER)
			goto out;

		lua_Number idx = lua_tonumber(L, -2);

		if (idx != (lua_Number)(lua_Integer)idx)
			goto out;

		/* We only allow INT_MAX-1 keys to avoid overflows */
		if (idx <= 0 || idx >= INT_MAX)
			goto out;

		if (idx > max)
			max = idx;

		lua_pop(L, 1);
	}

	/* check for holes */
	for (int i = 1; i <= max; i++) {
		lua_rawgeti(L, index, i);
		int isnil = lua_isnil(L, -1);
		lua_pop(L, 1);

		if (isnil)
			return -1;
	}

	return max;

out:
	lua_pop(L, 2);
	return -1;
}

struct json_object * lua_jsonc_tojson(lua_State *L, int index) {
	lua_Number nd, ni;
	struct json_object *obj;
	const char *key;
	int i, max;

	switch (lua_type(L, index)) {
	case LUA_TTABLE:
		max = lua_jsonc_lua_test_array(L, index);

		if (max >= 0) {
			obj = json_object_new_array();

			if (!obj)
				return NULL;

			for (i = 1; i <= max; i++) {
				lua_rawgeti(L, index, i);

				json_object_array_put_idx(
					obj, i - 1, lua_jsonc_tojson(L, lua_gettop(L))
				);

				lua_pop(L, 1);
			}

			return obj;
		}

		obj = json_object_new_object();

		if (!obj)
			return NULL;

		lua_pushnil(L);

		while (lua_next(L, index)) {
			lua_pushvalue(L, -2);
			key = lua_tostring(L, -1);

			if (key)
				json_object_object_add(
					obj, key, lua_jsonc_tojson(L, lua_gettop(L) - 1)
				);

			lua_pop(L, 2);
		}

		return obj;

	case LUA_TNIL:
		return NULL;

	case LUA_TBOOLEAN:
		return json_object_new_boolean(lua_toboolean(L, index));

	case LUA_TNUMBER:
		nd = lua_tonumber(L, index);
		ni = lua_tointeger(L, index);

		if (nd == ni)
			return json_object_new_int(nd);

		return json_object_new_double(nd);

	case LUA_TSTRING:
		return json_object_new_string(lua_tostring(L, index));
	}

	return NULL;
}

static int lua_jsonc_load(lua_State *L) {
	const char *filename = luaL_checkstring(L, 1);

	struct json_object *obj = json_object_from_file(filename);
	lua_jsonc_push_json(L, obj);
	json_object_put(obj);

	return 1;
}

static int lua_jsonc_parse(lua_State *L) {
	const char *input = luaL_checkstring(L, 1);

	struct json_object *obj = json_tokener_parse(input);
	lua_jsonc_push_json(L, obj);
	json_object_put(obj);

	return 1;
}

static int lua_jsonc_stringify(lua_State *L) {
	struct json_object *obj = lua_jsonc_tojson(L, 1);
	int pretty = lua_toboolean(L, 2);
	int flags = 0;

	if (pretty)
		flags |= JSON_C_TO_STRING_PRETTY | JSON_C_TO_STRING_SPACED;

	lua_pushstring(L, json_object_to_json_string_ext(obj, flags));
	json_object_put(obj);
	return 1;
}


static const luaL_reg R[] = {
	{ "load", lua_jsonc_load },
	{ "parse", lua_jsonc_parse },
	{ "stringify", lua_jsonc_stringify },
	{}
};


int luaopen_jsonc(lua_State *L) {
	luaL_register(L, "jsonc", R);
	return 1;
}
