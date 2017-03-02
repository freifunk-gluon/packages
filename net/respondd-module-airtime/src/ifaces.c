#include <linux/nl80211.h>
#include <netlink/genl/genl.h>

#include "ifaces.h"
#include "netlink.h"

static int iface_dump_handler(struct nl_msg *msg, void *arg) {
	struct nlattr *tb[NL80211_ATTR_MAX + 1];
	struct genlmsghdr *gnlh = nlmsg_data(nlmsg_hdr(msg));
	int wiphy;
	struct iface_list **last_next;

	nla_parse(tb, NL80211_ATTR_MAX, genlmsg_attrdata(gnlh, 0), genlmsg_attrlen(gnlh, 0), NULL);

	if (!tb[NL80211_ATTR_WIPHY] || !tb[NL80211_ATTR_IFINDEX])
		goto skip;

	wiphy = nla_get_u32(tb[NL80211_ATTR_WIPHY]);
	for (last_next = arg; *last_next != NULL; last_next = &(*last_next)->next) {
		if ((*last_next)->wiphy == wiphy)
			goto skip;
	}
	*last_next = malloc(sizeof(**last_next));
	if (!*last_next)
		goto skip;
	(*last_next)->next = NULL;
	(*last_next)->ifx = nla_get_u32(tb[NL80211_ATTR_IFINDEX]);
	(*last_next)->wiphy = wiphy;

skip:
	return NL_SKIP;
}

struct iface_list *get_ifaces() {
	struct iface_list *ifaces = NULL;
	nl_send_dump(&iface_dump_handler, &ifaces, NL80211_CMD_GET_INTERFACE, 0);
	return ifaces;
}
