#include <linux/nl80211.h>
#include <netlink/genl/genl.h>

#include "ifaces.h"
#include "netlink.h"


//https://github.com/torvalds/linux/blob/master/include/uapi/linux/nl80211.h#L4031
static const int chanwidth[NL80211_SURVEY_INFO_MAX + 1] = {
	[NL80211_CHAN_WIDTH_20_NOHT] = 20,
	[NL80211_CHAN_WIDTH_20] = 20,
	[NL80211_CHAN_WIDTH_40] = 40,
	[NL80211_CHAN_WIDTH_80] = 80,
	[NL80211_CHAN_WIDTH_80P80] = 160,
	[NL80211_CHAN_WIDTH_160] = 160,
	[NL80211_CHAN_WIDTH_5] = 5,
	[NL80211_CHAN_WIDTH_10] = 10,
};

static int iface_dump_handler(struct nl_msg *msg, void *arg) {
	struct nlattr *tb[NL80211_ATTR_MAX + 1];
	struct genlmsghdr *gnlh = nlmsg_data(nlmsg_hdr(msg));
	struct iface_list **last_next;

	nla_parse(tb, NL80211_ATTR_MAX, genlmsg_attrdata(gnlh, 0), genlmsg_attrlen(gnlh, 0), NULL);

	if (!tb[NL80211_ATTR_WIPHY] || !tb[NL80211_ATTR_IFINDEX])
		goto skip;

	#ifdef GLUON
	if(nla_strcmp(tb[NL80211_ATTR_IFNAME], "client") == -1 || nla_strcmp(tb[NL80211_ATTR_IFNAME], "ibss") == -1 || nla_strcmp(tb[NL80211_ATTR_IFNAME], "mesh") == -1)
	  goto skip;
	#endif

	// TODO fix add to head list - instatt find last item
	for (last_next = arg; *last_next != NULL; last_next = &(*last_next)->next) {}

	*last_next = malloc(sizeof(**last_next));
	if (!*last_next)
		goto skip;
	(*last_next)->next = NULL;
	(*last_next)->wiphy = nla_get_u32(tb[NL80211_ATTR_WIPHY]);
	(*last_next)->ifx = nla_get_u32(tb[NL80211_ATTR_IFINDEX]);
	(*last_next)->frequency = tb[NL80211_ATTR_WIPHY_FREQ] ? nla_get_u32(tb[NL80211_ATTR_WIPHY_FREQ]) : 0;
	(*last_next)->txpower = tb[NL80211_ATTR_WIPHY_TX_POWER_LEVEL] ? nla_get_u32(tb[NL80211_ATTR_WIPHY_TX_POWER_LEVEL]) : 0;
	(*last_next)->type = tb[NL80211_ATTR_IFTYPE] ? nla_get_u32(tb[NL80211_ATTR_IFTYPE]) : 0;

	if(tb[NL80211_ATTR_MAC]) {
		mac_addr_n2a((*last_next)->mac_addr, nla_data(tb[NL80211_ATTR_MAC]));
	}

	int chanwidth_id = nla_get_u32(tb[NL80211_ATTR_CHANNEL_WIDTH]);

	if(chanwidth[chanwidth_id])
		(*last_next)->chanwidth = tb[NL80211_ATTR_CHANNEL_WIDTH] ? chanwidth[chanwidth_id] : 0;

skip:
	return NL_SKIP;
}

struct iface_list *get_ifaces() {
	struct iface_list *ifaces = NULL;
	nl_send_dump(&iface_dump_handler, &ifaces, NL80211_CMD_GET_INTERFACE, 0);
	return ifaces;
}
