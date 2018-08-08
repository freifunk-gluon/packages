#pragma once

struct iface_list {
	int wiphy;
	int ifx;
	int frequency;
	int txpower;
	struct iface_list *next;
};

__attribute__((visibility("hidden"))) struct iface_list *get_ifaces();
