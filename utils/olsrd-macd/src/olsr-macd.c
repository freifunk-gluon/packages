/* SPDX-FileCopyrightText: 2021-2023 Maciej Krüger <maciej@xeredo.it> */
/* SPDX-License-Identifier: BSD-2-Clause */

/*
	Remembers the MAC each OLSR neighbour address was last seen with, read
	off the OLSR traffic on a packet socket, and answers over a unix socket.
	olsrd itself only sees UDP and knows no MACs.
*/

#include "olsr-macd.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include <arpa/inet.h>
#include <net/ethernet.h>
#include <net/if.h>
#include <netinet/ip.h>
#include <netinet/ip6.h>
#include <netpacket/packet.h>
#include <sys/socket.h>
#include <sys/un.h>

#include <linux/filter.h>

#include <json-c/json.h>
#include <libubox/uloop.h>
#include <libubox/utils.h>

/* port olsrd sends its packets to, see OlsrPort in 360-gluon-mesh-olsrd-setup-intf */
#define OLSR_PORT 698

/** seconds an address is remembered */
#define ENTRY_TIMEOUT 300

/** upper bound on remembered addresses */
#define MAX_ENTRIES 1024

#define REQUEST_LEN 128
#define PACKET_LEN 128

struct entry {
	unsigned int ifindex;
	int family;
	unsigned char addr[16];
	char mac[18];
	time_t seen;

	struct entry *next;
};

static struct entry *entries;
static size_t entry_count;

static size_t addr_len(int family) {
	return family == AF_INET ? 4 : 16;
}

static time_t now(void) {
	struct timespec ts;

	clock_gettime(CLOCK_MONOTONIC, &ts);

	return ts.tv_sec;
}

static void entries_expire(void) {
	struct entry **cur = &entries;
	time_t deadline = now() - ENTRY_TIMEOUT;

	while (*cur) {
		struct entry *entry = *cur;

		if (entry->seen < deadline) {
			*cur = entry->next;
			free(entry);
			entry_count--;
		} else {
			cur = &entry->next;
		}
	}
}

/** addr was last seen from mac */
static void entry_update(unsigned int ifindex, int family, const void *addr, const unsigned char *mac) {
	for (struct entry *entry = entries; entry; entry = entry->next) {
		if (entry->ifindex != ifindex || entry->family != family)
			continue;

		if (memcmp(entry->addr, addr, addr_len(family)))
			continue;

		snprintf(entry->mac, sizeof(entry->mac), "%02x:%02x:%02x:%02x:%02x:%02x",
			mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
		entry->seen = now();

		return;
	}

	entries_expire();

	if (entry_count >= MAX_ENTRIES)
		return;

	struct entry *entry = calloc(1, sizeof(*entry));
	if (!entry)
		return;

	entry->ifindex = ifindex;
	entry->family = family;
	memcpy(entry->addr, addr, addr_len(family));
	snprintf(entry->mac, sizeof(entry->mac), "%02x:%02x:%02x:%02x:%02x:%02x",
		mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
	entry->seen = now();

	entry->next = entries;
	entries = entry;
	entry_count++;
}

/** MAC of an address, NULL if unknown */
static const char * entry_lookup(const char *ifname, const char *ip) {
	unsigned int ifindex = if_nametoindex(ifname);
	if (!ifindex)
		return NULL;

	unsigned char addr[16];
	int family = AF_INET;

	if (inet_pton(AF_INET, ip, addr) != 1) {
		family = AF_INET6;

		if (inet_pton(AF_INET6, ip, addr) != 1)
			return NULL;
	}

	time_t deadline = now() - ENTRY_TIMEOUT;

	for (struct entry *entry = entries; entry; entry = entry->next) {
		if (entry->ifindex != ifindex || entry->family != family || entry->seen < deadline)
			continue;

		if (!memcmp(entry->addr, addr, addr_len(family)))
			return entry->mac;
	}

	return NULL;
}

/** { "<ifname>": { "<ip>": "<mac>" } } */
static json_object * entries_dump(void) {
	json_object *out = json_object_new_object();
	if (!out)
		return NULL;

	entries_expire();

	for (struct entry *entry = entries; entry; entry = entry->next) {
		char ifname[IF_NAMESIZE];
		char ip[INET6_ADDRSTRLEN];

		if (!if_indextoname(entry->ifindex, ifname))
			continue;

		if (!inet_ntop(entry->family, entry->addr, ip, sizeof(ip)))
			continue;

		json_object *intf = json_object_object_get(out, ifname);

		if (!intf) {
			intf = json_object_new_object();
			json_object_object_add(out, ifname, intf);
		}

		json_object_object_add(intf, ip, json_object_new_string(entry->mac));
	}

	return out;
}

/* --- packet socket --- */

static void handle_packet(struct uloop_fd *fd, unsigned int events);

static struct uloop_fd packet_fd[2] = {
	{ .cb = handle_packet },
	{ .cb = handle_packet },
};

static void handle_packet(struct uloop_fd *fd, unsigned int events) {
	unsigned char buf[PACKET_LEN];

	while (true) {
		struct sockaddr_ll from;
		socklen_t fromlen = sizeof(from);

		ssize_t len = recvfrom(fd->fd, buf, sizeof(buf), MSG_DONTWAIT,
			(struct sockaddr *)&from, &fromlen);

		if (len < 0)
			return;

		/* skip our own packets */
		if (from.sll_pkttype == PACKET_OUTGOING || from.sll_halen != ETH_ALEN)
			continue;

		if (len < (ssize_t)ETH_HLEN)
			continue;

		/* skip the ethernet header */
		const unsigned char *packet = buf + ETH_HLEN;
		ssize_t packet_len = len - ETH_HLEN;

		if (ntohs(from.sll_protocol) == ETH_P_IP) {
			const struct iphdr *ip = (const struct iphdr *)packet;

			if (packet_len < (ssize_t)sizeof(*ip))
				continue;

			entry_update(from.sll_ifindex, AF_INET, &ip->saddr, from.sll_addr);
		} else {
			const struct ip6_hdr *ip6 = (const struct ip6_hdr *)packet;

			if (packet_len < (ssize_t)sizeof(*ip6))
				continue;

			entry_update(from.sll_ifindex, AF_INET6, &ip6->ip6_src, from.sll_addr);
		}
	}
}

/* plain ethernet header only, no VLAN tags (same as olsrd's arprefresh) */
static struct sock_filter filter_ipv4[] = {
	{ BPF_LD  | BPF_H | BPF_ABS, 0, 0, 12 },          /* ethertype */
	{ BPF_JMP | BPF_JEQ | BPF_K, 0, 8, ETH_P_IP },
	{ BPF_LD  | BPF_B | BPF_ABS, 0, 0, 23 },          /* protocol */
	{ BPF_JMP | BPF_JEQ | BPF_K, 0, 6, IPPROTO_UDP },
	{ BPF_LD  | BPF_H | BPF_ABS, 0, 0, 20 },          /* fragment offset */
	{ BPF_JMP | BPF_JSET | BPF_K, 4, 0, 0x1fff },
	{ BPF_LDX | BPF_B | BPF_MSH, 0, 0, 14 },          /* IP header length */
	{ BPF_LD  | BPF_H | BPF_IND, 0, 0, 16 },          /* destination port */
	{ BPF_JMP | BPF_JEQ | BPF_K, 0, 1, OLSR_PORT },
	{ BPF_RET | BPF_K, 0, 0, 0xffffffff },
	{ BPF_RET | BPF_K, 0, 0, 0 },
};

static struct sock_filter filter_ipv6[] = {
	{ BPF_LD  | BPF_H | BPF_ABS, 0, 0, 12 },          /* ethertype */
	{ BPF_JMP | BPF_JEQ | BPF_K, 0, 5, ETH_P_IPV6 },
	{ BPF_LD  | BPF_B | BPF_ABS, 0, 0, 20 },          /* next header */
	{ BPF_JMP | BPF_JEQ | BPF_K, 0, 3, IPPROTO_UDP },
	{ BPF_LD  | BPF_H | BPF_ABS, 0, 0, 56 },          /* destination port */
	{ BPF_JMP | BPF_JEQ | BPF_K, 0, 1, OLSR_PORT },
	{ BPF_RET | BPF_K, 0, 0, 0xffffffff },
	{ BPF_RET | BPF_K, 0, 0, 0 },
};

/** packet socket for the OLSR traffic of one address family */
static int packet_socket(int protocol, struct sock_filter *filter, unsigned short filter_len) {
	/* SOCK_RAW keeps the ethernet header for the filter */
	int fd = socket(PF_PACKET, SOCK_RAW | SOCK_NONBLOCK | SOCK_CLOEXEC, htons(protocol));
	if (fd < 0) {
		perror("olsr-macd: packet socket");
		return -1;
	}

	struct sock_fprog prog = {
		.len = filter_len,
		.filter = filter,
	};

	if (setsockopt(fd, SOL_SOCKET, SO_ATTACH_FILTER, &prog, sizeof(prog))) {
		perror("olsr-macd: SO_ATTACH_FILTER");
		close(fd);
		return -1;
	}

	return fd;
}

/* --- unix socket --- */

/*
	one line requests, plain text answers:

		dump                    -> json of everything
		resolve <ifname> <ip>   -> the MAC, or an empty line
*/
static void handle_request(int fd) {
	char buf[REQUEST_LEN];
	size_t len = 0;

	while (len < sizeof(buf) - 1) {
		ssize_t read_len = recv(fd, buf + len, sizeof(buf) - len - 1, 0);

		if (read_len <= 0)
			break;

		len += read_len;

		if (memchr(buf, '\n', len))
			break;
	}

	buf[len] = '\0';
	buf[strcspn(buf, "\r\n")] = '\0';

	if (!strcmp(buf, "dump")) {
		json_object *dump = entries_dump();

		if (dump) {
			const char *str = json_object_to_json_string_ext(dump, JSON_C_TO_STRING_PLAIN);
			send(fd, str, strlen(str), MSG_NOSIGNAL);
			json_object_put(dump);
		}

		return;
	}

	char ifname[32], ip[64];

	if (sscanf(buf, "resolve %31s %63s", ifname, ip) == 2) {
		const char *mac = entry_lookup(ifname, ip);

		if (mac)
			send(fd, mac, strlen(mac), MSG_NOSIGNAL);

		send(fd, "\n", 1, MSG_NOSIGNAL);
	}
}

static void handle_connection(struct uloop_fd *fd, unsigned int events) {
	while (true) {
		int client = accept(fd->fd, NULL, NULL);

		if (client < 0)
			return;

		/* no waiting for slow clients */
		struct timeval timeout = { .tv_sec = 1 };
		setsockopt(client, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));

		handle_request(client);

		close(client);
	}
}

static struct uloop_fd listen_fd = { .cb = handle_connection };

static int listen_socket(const char *path) {
	int fd = socket(AF_UNIX, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, 0);
	if (fd < 0) {
		perror("olsr-macd: unix socket");
		return -1;
	}

	struct sockaddr_un addr = {
		.sun_family = AF_UNIX,
	};

	if (strlen(path) >= sizeof(addr.sun_path)) {
		fprintf(stderr, "olsr-macd: socket path %s is too long\n", path);
		close(fd);
		return -1;
	}

	strcpy(addr.sun_path, path);
	unlink(path);

	if (bind(fd, (struct sockaddr *)&addr, sizeof(addr)) || listen(fd, 8)) {
		perror("olsr-macd: bind");
		close(fd);
		return -1;
	}

	return fd;
}

int main(void) {
	uloop_init();

	packet_fd[0].fd = packet_socket(ETH_P_IP, filter_ipv4, ARRAY_SIZE(filter_ipv4));
	packet_fd[1].fd = packet_socket(ETH_P_IPV6, filter_ipv6, ARRAY_SIZE(filter_ipv6));

	if (packet_fd[0].fd < 0 || packet_fd[1].fd < 0)
		return 1;

	listen_fd.fd = listen_socket(OLSR_MACD_SOCKET);
	if (listen_fd.fd < 0)
		return 1;

	uloop_fd_add(&packet_fd[0], ULOOP_READ);
	uloop_fd_add(&packet_fd[1], ULOOP_READ);
	uloop_fd_add(&listen_fd, ULOOP_READ);

	uloop_run();
	uloop_done();

	unlink(OLSR_MACD_SOCKET);

	return 0;
}
