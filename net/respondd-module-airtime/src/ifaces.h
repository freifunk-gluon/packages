#pragma once

struct iface_list {
	int ifx;
	int wiphy;
	struct iface_list *next;
};

__attribute__((visibility("hidden"))) struct iface_list *get_ifaces();
