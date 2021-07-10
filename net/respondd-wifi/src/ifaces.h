#pragma once

#include <stdbool.h>

struct iface_list {
	int ifx;
	char mac_addr[20];
	int type;
	int frequency;
	struct iface_list *next;
};

__attribute__((visibility("hidden"))) struct iface_list *get_ifaces();
