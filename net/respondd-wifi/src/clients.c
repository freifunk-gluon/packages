#include <linux/nl80211.h>
#include <netlink/genl/genl.h>

#include "netlink.h"
#include "clients.h"

static int station_client_handler(struct nl_msg *msg, void *arg) {
	int *count = (int *) arg;

	(*count)++;

	return NL_SKIP;
}
bool get_client_counts(int *count, int ifx) {
	return nl_send_dump(station_client_handler, count, NL80211_CMD_GET_STATION, ifx);
}
