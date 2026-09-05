/* SPDX-FileCopyrightText: 2021-2023 Maciej Krüger <maciej@xeredo.it> */
/* SPDX-License-Identifier: BSD-2-Clause */

#include <dirent.h>
#include <errno.h>
#include <string.h>

#include <lua.h>
#include <lauxlib.h>
#include <lua-jsonc.h>

#include <libolsrdhelper.h>

#define OLSRD "gluon.olsrd"

static int find_module_version (lua_State *L) {
	const char *mod = luaL_checkstring(L, 1);

	DIR *d = opendir("/usr/lib");

	if (d == NULL)
		return luaL_error(L, "cannot open /usr/lib: %s", strerror(errno));

	struct dirent *entry;
	while ((entry = readdir(d)) != NULL) {
		if (entry->d_type == DT_REG && !strncmp(mod, entry->d_name, strlen(mod))) {
			lua_pushstring(L, entry->d_name);
			closedir(d);
			return 1;
		}
	}

	closedir(d);
	return luaL_error(L, "mod %s not found", mod);
}

/** Checks the address family argument, either 4 or 6 */
static int check_ipv (lua_State *L, int index) {
	int ipv = luaL_checkinteger(L, index);

	if (ipv != OLSR_IPV4 && ipv != OLSR_IPV6)
		luaL_error(L, "invalid address family %d, expected 4 or 6", ipv);

	return ipv;
}

static int lua_get_nodeinfo (lua_State *L) {
	int ipv = check_ipv(L, 1);
	const char *query = luaL_checkstring(L, 2);

	json_object *resp;

	if (olsr_get_nodeinfo(ipv, query, &resp))
		return luaL_error(L, "get_nodeinfo(%d, %s) failed", ipv, query);

	lua_jsonc_push_json(L, resp);

	return 1;
}

static int lua_get_neigh (lua_State *L) {
	int ipv = check_ipv(L, 1);

	json_object *resp = olsr_get_neigh(ipv);

	if (!resp)
		return luaL_error(L, "get_neigh(%d) failed", ipv);

	lua_jsonc_push_json(L, resp);

	return 1;
}

static void push_daemon (lua_State *L, const struct olsr_daemon_info *daemon, const char *name) {
	lua_newtable(L);

	lua_pushboolean(L, daemon->enabled);
	lua_setfield(L, -2, "enabled");

	lua_pushboolean(L, daemon->running);
	lua_setfield(L, -2, "running");

	lua_setfield(L, -2, name);
}

static int lua_get_info (lua_State *L) {
	struct olsr_info info;

	if (olsr_get_info(&info))
		return luaL_error(L, "get_info() failed");

	lua_newtable(L);

	push_daemon(L, &info.olsr4, olsr_name(OLSR_IPV4));
	push_daemon(L, &info.olsr6, olsr_name(OLSR_IPV6));

	return 1;
}

static const luaL_reg olsrd_methods[] = {
	{ "find_module_version", find_module_version },

	{ "get_info", lua_get_info },

	{ "get_nodeinfo", lua_get_nodeinfo },
	{ "get_neigh", lua_get_neigh },
	{ }
};

int luaopen_gluon_olsrd(lua_State *L)
{
	luaL_register(L, OLSRD, olsrd_methods);

	return 1;
}
