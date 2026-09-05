/* SPDX-FileCopyrightText: 2021-2023 Maciej Krüger <maciej@xeredo.it> */
/* SPDX-License-Identifier: BSD-2-Clause */

#pragma once

#include <json-c/json.h>
#include <libubox/uclient.h>

#include <stdbool.h>

/* one daemon per family, olsrd (IPv4) and olsrd6 (IPv6), each with its own jsoninfo port */
#define OLSR_IPV4 4
#define OLSR_IPV6 6

struct olsr_daemon_info {
	bool enabled;
	bool running;
};

struct olsr_info {
	struct olsr_daemon_info olsr4;
	struct olsr_daemon_info olsr6;
};

/** daemon of the address family, NULL if unknown */
static inline const struct olsr_daemon_info * olsr_daemon(const struct olsr_info *info, int ipv) {
	switch (ipv) {
	case OLSR_IPV4:
		return &info->olsr4;
	case OLSR_IPV6:
		return &info->olsr6;
	default:
		return NULL;
	}
}

/** daemon name as used in respondd output */
static inline const char * olsr_name(int ipv) {
	return ipv == OLSR_IPV4 ? "olsr4" : "olsr6";
}

/* site is the gluon site config, prefix4 enables olsrd and prefix6 olsrd6, the caller keeps its reference */
int olsr_get_info(json_object *site, struct olsr_info *out);

int olsr_get_nodeinfo(int ipv, const char *path, json_object **out);

struct json_object * olsr_get_neigh(int ipv);
struct json_object * olsr_get_merged_neighs(const struct olsr_info *info);

// generic socket helpers

/* out is optional, without it the raw fd is returned */
int socket_request(const char *path, const char *cmd, char **out);

json_object * socket_request_json(const char *path, const char *cmd);

// macros for json c

#define J_OUT(x) json_object *out = json_object_get((x));	\
	json_object_put(resp);					\
	return out;

#define J_OGET(obj, key) json_object_get(json_object_object_get(obj, key))

#define J_OCPY(dst, src, key) json_object_object_add(dst, key, json_object_get(json_object_object_get(src, key)))

#define J_OCPY2(dst, src, dkey, skey) json_object_object_add(dst, dkey, json_object_get(json_object_object_get(src, skey)))
