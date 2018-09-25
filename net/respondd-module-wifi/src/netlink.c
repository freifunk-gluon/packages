#include <inttypes.h>

#include <linux/nl80211.h>
#include <linux/if_ether.h>
#include <netlink/genl/genl.h>
#include <netlink/genl/ctrl.h>

#include "netlink.h"


void mac_addr_n2a(char *mac_addr, unsigned char *arg) {
	sprintf(mac_addr, "%02x:%02x:%02x:%02x:%02x:%02x", arg[0], arg[1], arg[2], arg[3], arg[4], arg[5]);
}


bool nl_send_dump(nl_recvmsg_msg_cb_t cb, void *cb_arg, int cmd, uint32_t cmd_arg) {
	bool ok = false;
	int ret;
	int ctrl;
	struct nl_sock *sk = NULL;
	struct nl_msg *msg = NULL;


#define ERR(...) { fprintf(stderr, "respondd-module-wifi: " __VA_ARGS__); goto out; }

	sk = nl_socket_alloc();
	if (!sk)
		ERR("nl_socket_alloc() failed\n");

	ret = genl_connect(sk);
	if (ret < 0)
		ERR("genl_connect() returned %d\n", ret);

	ctrl = genl_ctrl_resolve(sk, NL80211_GENL_NAME);
	if (ctrl < 0)
		ERR("genl_ctrl_resolve() returned %d\n", ctrl);

	ret = nl_socket_modify_cb(sk, NL_CB_VALID, NL_CB_CUSTOM, cb, cb_arg);
	if (ret != 0)
		ERR("nl_socket_modify_cb() returned %d\n", ret);

	msg = nlmsg_alloc();
	if (!msg)
		ERR("nlmsg_alloc() failed\n");

	if (!genlmsg_put(msg, 0, 0, ctrl, 0, NLM_F_DUMP, cmd, 0))
		ERR("genlmsg_put() failed while putting cmd %d\n", ret, cmd);

	if (cmd_arg != 0)
		NLA_PUT_U32(msg, NL80211_ATTR_IFINDEX, cmd_arg);

	ret = nl_send_auto_complete(sk, msg);
	if (ret < 0)
		ERR("nl_send_auto() returned %d while sending cmd %d with cmd_arg=%"PRIu32"\n", ret, cmd, cmd_arg);

	ret = nl_recvmsgs_default(sk);
	if (ret < 0)
		ERR("nl_recv_msgs_default() returned %d while receiving cmd %d with cmd_arg=%"PRIu32"\n", ret, cmd, cmd_arg);

#undef ERR

	ok = true;

nla_put_failure:
out:
	if (msg)
		nlmsg_free(msg);

	if (sk)
		nl_socket_free(sk);

	return ok;
}
