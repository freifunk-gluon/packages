#include <linux/nl80211.h>
#include <netlink/genl/genl.h>
#include <netlink/genl/ctrl.h>

#include "netlink.h"

bool nl_send_dump(nl_recvmsg_msg_cb_t cb, void *cb_arg, int cmd, uint32_t cmd_arg) {
	bool ok = false;
	int ctrl;
	struct nl_sock *sk = NULL;
	struct nl_msg *msg = NULL;


#define CHECK(x) { if (!(x)) { fprintf(stderr, "%s: error on line %d\n", __FILE__, __LINE__); goto out; } }

	CHECK(sk = nl_socket_alloc());
	CHECK(genl_connect(sk) >= 0);

	CHECK(ctrl = genl_ctrl_resolve(sk, NL80211_GENL_NAME));
	CHECK(nl_socket_modify_cb(sk, NL_CB_VALID, NL_CB_CUSTOM, cb, cb_arg) == 0);
	CHECK(msg = nlmsg_alloc());
	CHECK(genlmsg_put(msg, 0, 0, ctrl, 0, NLM_F_DUMP, cmd, 0));

	if (cmd_arg != 0)
		NLA_PUT_U32(msg, NL80211_ATTR_IFINDEX, cmd_arg);

	CHECK(nl_send_auto_complete(sk, msg) >= 0);
	CHECK(nl_recvmsgs_default(sk) >= 0);

#undef CHECK

	ok = true;

nla_put_failure:
out:
	if (msg)
		nlmsg_free(msg);

	if (sk)
		nl_socket_free(sk);

	return ok;
}
