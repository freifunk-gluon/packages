#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <netlink/handlers.h>

__attribute__((visibility("hidden"))) void mac_addr_n2a(char *mac_addr, unsigned char *arg);
__attribute__((visibility("hidden"))) bool nl_send_dump(nl_recvmsg_msg_cb_t cb, void *cb_arg, int cmd, uint32_t cmd_arg);
