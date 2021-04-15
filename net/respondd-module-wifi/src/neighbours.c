#include <linux/nl80211.h>
#include <netlink/genl/genl.h>

#include "netlink.h"
#include "neighbours.h"

static const char * neighbours_names[NL80211_STA_INFO_MAX + 1] = {
	[NL80211_STA_INFO_SIGNAL] = "signal",
	[NL80211_STA_INFO_INACTIVE_TIME] = "inactive",
};

static int station_neighbours_handler(struct nl_msg *msg, void *arg) {
	struct json_object *neighbour, *json = (struct json_object *) arg;

	neighbour = json_object_new_object();
	if (!neighbour)
		goto abort;

	struct nlattr *tb[NL80211_ATTR_MAX + 1];

	struct genlmsghdr *gnlh = nlmsg_data(nlmsg_hdr(msg));
	struct nlattr *station_info = nla_find(genlmsg_attrdata(gnlh, 0), genlmsg_attrlen(gnlh, 0), NL80211_ATTR_STA_INFO);

	nla_parse(tb, NL80211_ATTR_MAX, genlmsg_attrdata(gnlh, 0), genlmsg_attrlen(gnlh, 0), NULL);

	if (!station_info) {
		fputs("respondd-module-wifi: station data missing in netlink message\n", stderr);
		json_object_put(neighbour);
		goto abort;
	}

	char mac_addr[20];

	if (!tb[NL80211_ATTR_MAC]) {
		json_object_put(neighbour);
		goto abort;
	}
	mac_addr_n2a(mac_addr, nla_data(tb[NL80211_ATTR_MAC]));

	int rem;
	struct nlattr *nla;
	nla_for_each_nested(nla, station_info, rem) {
		int type = nla_type(nla);

		if (type > NL80211_STA_INFO_MAX)
			continue;

		if (!neighbours_names[type])
			continue;

		struct json_object *data_json = NULL;
		switch (nla_len(nla)) {
			case sizeof(uint64_t):
				data_json = json_object_new_int64(nla_get_u64(nla));
				break;
			case sizeof(uint32_t):
				data_json = json_object_new_int(nla_get_u32(nla));
				break;
			case sizeof(uint8_t):
					data_json = json_object_new_int(nla_get_u8(nla));
				break;
			default:
				fprintf(stderr, "respondd-module-wifi: Unexpected NL attribute length: %d\n", nla_len(nla));
		}
		if (data_json)
			json_object_object_add(neighbour, neighbours_names[type], data_json);
	}
	json_object_object_add(json, mac_addr, neighbour);

abort:
	return NL_SKIP;
}

bool get_neighbours(struct json_object *result, int ifx) {
	struct json_object *neighbours;
	neighbours = json_object_new_object();
	if (!neighbours)
		return false;
  if(!nl_send_dump(station_neighbours_handler, neighbours, NL80211_CMD_GET_STATION, ifx)) {
		json_object_put(neighbours);
		return false;
	}
	json_object_object_add(result, "neighbours", neighbours);
	return true;

}
