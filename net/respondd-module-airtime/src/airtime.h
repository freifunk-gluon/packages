#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <json-c/json.h>

__attribute__((visibility("hidden"))) bool get_airtime(struct json_object *result, int ifx);
