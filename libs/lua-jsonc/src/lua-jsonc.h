#ifndef LUA_JSONC_H_
#define LUA_JSONC_H_

#include <json-c/json.h>
#include <lua.h>

void lua_jsonc_push_json(lua_State *L, struct json_object *obj);
struct json_object * lua_jsonc_tojson(lua_State *L, int index);

#endif /* LUA_JSONC_H_ */
