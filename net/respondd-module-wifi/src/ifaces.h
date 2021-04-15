#pragma once

#include <stdbool.h>

struct iface_list {
	int wiphy;
	int ifx;
	char mac_addr[20];
	int type;
	int frequency;
	int chanwidth;
	int txpower;
	struct iface_list *next;
};

__attribute__((visibility("hidden"))) struct iface_list *get_ifaces();
