#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <json-c/json.h>

__attribute__((visibility("hidden")))
struct json_object * get_airtime(int ifx);
