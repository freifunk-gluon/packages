/* SPDX-FileCopyrightText: 2021-2023 Maciej Krüger <maciej@xeredo.it> */
/* SPDX-License-Identifier: BSD-2-Clause */

#include "libolsrdhelper.h"
#include "olsr-macd.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* merges one daemon's neighbours into out (keyed by MAC), shared values averaged, addresses kept as <name>_ip */
static void merge_neighs(json_object *out, json_object *neighs, const char *name) {
	json_object_object_foreach(neighs, mac, neighbour_original) {
		json_object *neighbour = json_object_object_get(out, mac);

		if (!neighbour) {
			neighbour = json_object_new_object();
			json_object_object_add(out, mac, neighbour);
		}

		json_object_object_foreach(neighbour_original, key, new) {
			json_object *cur = json_object_object_get(neighbour, key);

			if (!strcmp(key, "tq") || !strcmp(key, "etx")) {
				if (cur) {
					json_object_object_add(
						neighbour,
						key,
						json_object_new_double(
							(json_object_get_double(cur) + json_object_get_double(new)) / 2
						)
					);
				} else {
					json_object_object_add(neighbour, key, json_object_get(new));
				}
			} else if (!strcmp(key, "ip")) {
				char ip_key[32];
				snprintf(ip_key, sizeof(ip_key), "%s_%s", name, key);

				json_object_object_add(neighbour, ip_key, json_object_get(new));
			} else if (!strcmp(key, "best")) {
				if (cur) {
					json_object_object_add(
						neighbour,
						"best",
						json_object_new_boolean(
							json_object_get_boolean(cur) || json_object_get_boolean(new)
						)
					);
				} else {
					json_object_object_add(neighbour, "best", json_object_get(new));
				}
			} else {
				json_object_object_add(neighbour, key, json_object_get(new));
			}
		}
	}
}

json_object * olsr_get_merged_neighs(void) {
	struct olsr_info info;

	if (olsr_get_info(&info))
		return NULL;

	json_object *out = json_object_new_object();
	if (!out)
		return NULL;

	static const int families[] = { OLSR_IPV6, OLSR_IPV4 };

	for (size_t i = 0; i < sizeof(families) / sizeof(families[0]); i++) {
		int ipv = families[i];

		if (!olsr_daemon(&info, ipv)->running)
			continue;

		json_object *neighs = olsr_get_neigh(ipv);
		if (!neighs) {
			json_object_put(out);
			return NULL;
		}

		merge_neighs(out, neighs, olsr_name(ipv));
		json_object_put(neighs);
	}

	return out;
}

/*
	links

	localIP	"10.12.11.43"
	remoteIP	"10.12.11.1"
	olsrInterface	"mesh-vpn"
	ifName	"mesh-vpn"
	validityTime	141239
	symmetryTime	123095
	asymmetryTime	25552910
	vtime	124000
	currentLinkStatus	"SYMMETRIC"
	previousLinkStatus	"SYMMETRIC"
	hysteresis	0
	pending	false
	lostLinkTime	0
	helloTime	0
	lastHelloTime	0
	seqnoValid	false
	seqno	0
	lossHelloInterval	3000
	lossTime	3595
	lossMultiplier	65536
	linkCost	1.084961
	linkQuality	1
	neighborLinkQuality	0.921
*/
struct json_object * olsr_get_neigh(int ipv) {
	json_object *resp;

	if (olsr_get_nodeinfo(ipv, "links", &resp))
		return NULL;

	json_object *out = NULL;

	json_object *links = json_object_object_get(resp, "links");
	if (!links)
		goto cleanup_resp;

	/* olsrd knows no MACs, olsr-macd does */
	json_object *macs = socket_request_json(OLSR_MACD_SOCKET, "dump");
	if (!macs)
		goto cleanup_resp;

	out = json_object_new_object();
	if (!out)
		goto cleanup_macs;

	size_t linkcount = json_object_array_length(links);

	for (size_t i = 0; i < linkcount; i++) {
		struct json_object *link = json_object_array_get_idx(links, i);
		if (!link)
			continue;

		const char *ifname = json_object_get_string(json_object_object_get(link, "ifName"));
		const char *ip = json_object_get_string(json_object_object_get(link, "remoteIP"));

		if (!ifname || !ip)
			continue;

		const char *mac = json_object_get_string(
			json_object_object_get(json_object_object_get(macs, ifname), ip));

		if (!mac)
			continue;

		struct json_object *neigh = json_object_new_object();
		if (!neigh)
			goto fail;

		J_OCPY2(neigh, link, "ifname", "ifName");
		J_OCPY2(neigh, link, "ip", "remoteIP");

		// set this if we detect peer in hna is doing gw
		json_object_object_add(neigh, "best", json_object_new_boolean(0));

		const double quality =
			json_object_get_double(json_object_object_get(link, "linkQuality")) *
			json_object_get_double(json_object_object_get(link, "neighborLinkQuality"));

		json_object_object_add(neigh, "tq", json_object_new_double(255 * quality));

		// no etx for a dead link
		if (quality > 0)
			json_object_object_add(neigh, "etx", json_object_new_double(1 / quality));

		json_object_object_add(out, mac, neigh);
	}

	goto cleanup_macs;

fail:
	json_object_put(out);
	out = NULL;
cleanup_macs:
	json_object_put(macs);
cleanup_resp:
	json_object_put(resp);

	return out;
}
