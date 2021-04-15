#pragma once

#include <stdbool.h>
#include <json-c/json.h>

__attribute__((visibility("hidden"))) bool get_neighbours(struct json_object *result, int ifx);
