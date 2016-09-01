/*
   Copyright (c) 2014-2015, Nils Schneider <nils@nilsschneider.net>
   Copyright (c) 2015-2016, Matthias Schiffer <mschiffer@universe-factory.net>
   All rights reserved.

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions are met:

   1. Redistributions of source code must retain the above copyright notice,
   this list of conditions and the following disclaimer.
   2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
   AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
   IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
   DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
   FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
   DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
   SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
   CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
   OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
   OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "respondd.h"

#include "miniz.c"

#include <json-c/json.h>

#include <alloca.h>
#include <dirent.h>
#include <dlfcn.h>
#include <inttypes.h>
#include <search.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <errno.h>

#include <arpa/inet.h>
#include <net/if.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>

#define SCHEDULE_LEN 8
#define REQUEST_MAXLEN 256

struct provider_list {
	struct provider_list *next;

	char *name;
	respondd_provider provider;
};

struct request_type {
	// points to the corresponding provider_list for this request in the chained list
	struct provider_list *providers;

	struct json_object *cache;
	uint64_t cache_time;
	int64_t cache_timeout;
};

struct request_task {
	struct request_task *next;
	int64_t scheduled_time;

	struct sockaddr_in6 client_addr;
	char request[REQUEST_MAXLEN];
};

struct request_schedule {
	int length;
	struct request_task *list_head;
};

static int64_t now;
static struct hsearch_data htab;


static struct json_object * merge_json(struct json_object *a, struct json_object *b);


static void usage() {
	puts("Usage:");
	puts("  respondd -h");
	puts("  respondd [-p <port>] [-g <group> -i <if0> [-i <if1> ..]] [-d <dir>]");
	puts("        -p <int>         port number to listen on");
	puts("        -g <ip6>         multicast group, e.g. ff02::2:1001");
	puts("        -i <string>      interface on which the group is joined");
	puts("        -t <int>         delay seconds before multicast responses (default: 10)");
	puts("        -d <string>      data provider directory (default: current directory)");
	puts("        -h               this help\n");
}

static void join_mcast(const int sock, const struct in6_addr addr, const char *iface) {
	struct ipv6_mreq mreq;

	mreq.ipv6mr_multiaddr = addr;
	mreq.ipv6mr_interface = if_nametoindex(iface);

	if (mreq.ipv6mr_interface == 0)
		goto error;

	if (setsockopt(sock, IPPROTO_IPV6, IPV6_JOIN_GROUP, &mreq, sizeof(mreq)) == -1)
		goto error;

	return;

 error:
	fprintf(stderr, "Could not join multicast group on %s: ", iface);
	perror(NULL);
}


static void update_time(void) {
	struct timespec tp;
	clock_gettime(CLOCK_MONOTONIC, &tp);

	now = (int64_t)tp.tv_sec * 1000 + tp.tv_nsec / 1000000;
}


/**
 * Merges two JSON objects
 *
 * On conflicts, object a will be preferred.
 *
 * Internally, this functions merges all entries from object a into object b,
 * so merging a small object a with a big object b is faster than vice-versa.
 */
static struct json_object * merge_json(struct json_object *a, struct json_object *b) {
	if (!json_object_is_type(a, json_type_object) || !json_object_is_type(b, json_type_object)) {
		json_object_put(b);
		return a;
	}

	json_object_object_foreach(a, key, val_a) {
		struct json_object *val_b;

		json_object_get(val_a);

		if (!json_object_object_get_ex(b, key, &val_b)) {
			json_object_object_add(b, key, val_a);
			continue;
		}

		json_object_get(val_b);

		json_object_object_add(b, key, merge_json(val_a, val_b));
	}

	json_object_put(a);
	return b;
}

static const struct respondd_provider_info * get_providers(const char *filename) {
	/*
	  Prefix the filename with "./" to open the module in the current directory
	  (dlopen looks in the standard library paths by default)
	*/
	char path[2 + strlen(filename) + 1];
	snprintf(path, sizeof(path), "./%s", filename);

	void *handle = dlopen(path, RTLD_NOW|RTLD_LOCAL);
	if (!handle)
		return NULL;

	const struct respondd_provider_info *ret = dlsym(handle, "respondd_providers");
	if (!ret) {
		dlclose(handle);
		return NULL;
	}

	return ret;
}

bool schedule_push_request(struct request_schedule *s, struct request_task *new_task) {
	if (s->length >= SCHEDULE_LEN)
		// schedule is full
		return false;

	if (!s->list_head || s->list_head->scheduled_time > new_task->scheduled_time) {
		new_task->next = s->list_head;
		s->list_head = new_task;
	} else {
		struct request_task *t;
		for (t = s->list_head; t && t->next; t = t->next) {
			if (t->next->scheduled_time > new_task->scheduled_time) {
				break;
			}
		}
		new_task->next = t->next;
		t->next = new_task;
	}

	s->length++;
	return true;
}

uint64_t schedule_idle_time(struct request_schedule *s) {
	if (!s->list_head)
		// nothing to do yet, wait nearly infinite time
		return 3600000;

	update_time();
	int64_t result = s->list_head->scheduled_time - now;

	if (result < 0)
		return 0;
	else
		return result;
}

struct request_task * schedule_pop_request(struct request_schedule *s) {
	if (!s->list_head)
		// schedule is empty
		return NULL;

	if (schedule_idle_time(s) > 0) {
		// nothing to do yet
		return NULL;
	}

	struct request_task *result = s->list_head;
	s->list_head = s->list_head->next;
	s->length--;

	return result;
}

static void load_cache_time(struct request_type *r, const char *name) {
	r->cache = NULL;
	r->cache_time = 0;
	r->cache_timeout = now;

	char filename[strlen(name) + 7];
	snprintf(filename, sizeof(filename), "%s.cache", name);

	FILE *f = fopen(filename, "r");
	if (!f)
		return;

	fscanf(f, "%"SCNu64, &r->cache_time);
	fclose(f);

}

static void add_provider(const char *name, const struct respondd_provider_info *provider) {
	ENTRY key = {
		.key = (char *)provider->request,
		.data = NULL,
	};
	ENTRY *entry;
	if (!hsearch_r(key, FIND, &entry, &htab)) {
		struct request_type *r = malloc(sizeof(*r));
		r->providers = NULL;
		load_cache_time(r, provider->request);

		key.data = r;
		if (!hsearch_r(key, ENTER, &entry, &htab)) {
			perror("hsearch_r");
			exit(EXIT_FAILURE);
		}
	}

	struct request_type *r = entry->data;

	struct provider_list *pentry = malloc(sizeof(*pentry));
	pentry->name = strdup(name);
	pentry->provider = provider->provider;

	struct provider_list **pos;
	for (pos = &r->providers; *pos; pos = &(*pos)->next) {
		if (strcmp(pentry->name, (*pos)->name) < 0)
			break;
	}

	pentry->next = *pos;
	*pos = pentry;
}

static void load_providers(void) {
	update_time();

	/* Maximum number of request types, might be made configurable in the future */
	if (!hcreate_r(32, &htab)) {
		perror("hcreate_r");
		exit(EXIT_FAILURE);
	}

	DIR *dir = opendir(".");
	if (!dir) {
		perror("opendir");
		exit(EXIT_FAILURE);
	}

	struct dirent *ent;
	while ((ent = readdir(dir)) != NULL) {
		if (ent->d_name[0] == '.')
			continue;

		size_t len = strlen(ent->d_name);

		if (len < 4)
			continue;

		if (strcmp(&ent->d_name[len-3], ".so"))
			continue;

		const struct respondd_provider_info *providers = get_providers(ent->d_name);
		if (!providers)
			continue;

		for (; providers->request; providers++)
			add_provider(ent->d_name, providers);
	}

	closedir(dir);
}

static struct json_object * eval_providers(struct provider_list *providers) {
	struct json_object *ret = json_object_new_object();

	for (; providers; providers = providers->next)
		ret = merge_json(providers->provider(), ret);

	return ret;
}

static struct json_object * single_request(char *type) {
	ENTRY key = {
		.key = type,
		.data = NULL,
	};
	ENTRY *entry;
	if (!hsearch_r(key, FIND, &entry, &htab))
		return NULL;

	struct request_type *r = entry->data;

	if (r->cache_time && now < r->cache_timeout)
		return json_object_get(r->cache);

	struct json_object *ret = eval_providers(r->providers);

	if (r->cache_time) {
		if (r->cache)
			json_object_put(r->cache);

		r->cache = json_object_get(ret);
		r->cache_timeout = now + r->cache_time;
	}

	return ret;
}

static struct json_object * multi_request(char *types) {
	struct json_object *ret = json_object_new_object();
	char *type, *saveptr;

	for (type = strtok_r(types, " ", &saveptr); type; type = strtok_r(NULL, " ", &saveptr)) {
		struct json_object *sub = single_request(type);
		if (sub)
			json_object_object_add(ret, type, sub);
	}

	return ret;
}

static struct json_object * handle_request(char *request, bool *compress) {
	if (!*request)
		return NULL;

	if (!strncmp(request, "GET ", 4)) {
		*compress = true;
		return multi_request(request+4);
	}
	else {
		*compress = false;
		return single_request(request);
	}
}

// Wait for an incomming request and schedule it.
//
// 1a. If the schedule is empty, we will wait an nearly infinite time.
// 1b. If we have scheduled requests, we only wait for incomming requests
//     until we reach the scheduling deadline.
// 2a. If the incomming request was sent to a multicast destination IPv6,
//     choose a random delay between 0 and max_multicast_delay milliseconds.
// 2b. If the incomming request was sent to a unicast destination, delay
//     is zero.
static void accept_request(struct request_schedule *schedule, int sock,
                           uint64_t max_multicast_delay) {
	char input[REQUEST_MAXLEN];
	ssize_t input_bytes;
	struct sockaddr_in6 addr;
	socklen_t addrlen = sizeof(addr);
	char control[256];
	struct in6_addr destaddr;
	struct cmsghdr *cmsg;

	// set timeout to the socket
	uint64_t timeout = schedule_idle_time(schedule);
	struct timeval t;
	t.tv_sec = timeout / 1000;
	t.tv_usec = (timeout % 1000) * 1000;

	if (setsockopt (sock, SOL_SOCKET, SO_RCVTIMEO, (char *)&t, sizeof(t)) < 0)
		perror("setsockopt failed\n");

	struct iovec iv = {
		.iov_base = input,
		.iov_len = sizeof(input) - 1
	};

	struct msghdr mh = {
		.msg_name = &addr,
		.msg_namelen = sizeof(addr),
		.msg_iov = &iv,
		.msg_iovlen = 1,
		.msg_control = control,
		.msg_controllen = sizeof(control)
	};

	input_bytes = recvmsg(sock, &mh, 0);

	// Timeout
	if (input_bytes < 0 && errno == EWOULDBLOCK)
		return;

	if (input_bytes < 0) {
		perror("recvmsg failed");
		exit(EXIT_FAILURE);
	}

	// determine destination address
	for (cmsg = CMSG_FIRSTHDR(&mh);	cmsg != NULL; cmsg = CMSG_NXTHDR(&mh, cmsg))
	{
		// ignore the control headers that don't match what we WARRANTIES
		if (cmsg->cmsg_level != IPPROTO_IPV6 || cmsg->cmsg_type != IPV6_PKTINFO)
			continue;

		struct in6_pktinfo *pi = CMSG_DATA(cmsg);
		destaddr = pi->ipi6_addr;
		break;
	}

	input[input_bytes] = 0;

	// only multicast requests are delayed
	update_time();
	int64_t delay;
	if (IN6_IS_ADDR_MULTICAST(&destaddr)) {
		delay = rand() % max_multicast_delay;
	} else {
		delay = 0;
	}

	struct request_task *new_task = malloc(sizeof(struct request_task));
	new_task->scheduled_time = now + delay;
	strcpy(new_task->request, input);
	memcpy(&new_task->client_addr, &addr, addrlen);

	if(!schedule_push_request(schedule, new_task)) {
		free(new_task);
	}
}

void send_response(int sock, struct json_object *result, bool compress,
                   struct sockaddr_in6 *addr) {
	const char *output = NULL;
	size_t output_bytes;

	const char *str = json_object_to_json_string_ext(result, JSON_C_TO_STRING_PLAIN);

	if (compress) {
		size_t str_bytes = strlen(str);

		mz_ulong compressed_bytes = mz_compressBound(str_bytes);
		unsigned char *compressed = alloca(compressed_bytes);

		if (!mz_compress(compressed, &compressed_bytes, (const unsigned char *)str, str_bytes)) {
			output = (const char*)compressed;
			output_bytes = compressed_bytes;
		}
	}
	else {
		output = str;
		output_bytes = strlen(str);
	}

	if (output) {
		if (sendto(sock, output, output_bytes, 0, addr, sizeof(struct sockaddr_in6)) < 0)
			perror("sendto failed");
	}

	json_object_put(result);
}

void serve_request(struct request_schedule *schedule, int sock) {
	struct request_task* task = schedule_pop_request(schedule);

	if (!task)
		return;

	bool compress;
	struct json_object *result = handle_request(task->request, &compress);

	if (!result)
		return;

	send_response(
		sock,
		result,
		compress,
		&task->client_addr
	);

	free(task);
}

int main(int argc, char **argv) {
	const int one = 1;

	int sock;
	struct sockaddr_in6 server_addr = {};
	struct in6_addr mgroup_addr;

	srand(time(NULL));

	sock = socket(PF_INET6, SOCK_DGRAM, 0);

	if (sock < 0) {
		perror("creating socket");
		exit(EXIT_FAILURE);
	}

	if (setsockopt(sock, IPPROTO_IPV6, IPV6_V6ONLY, &one, sizeof(one))) {
		perror("can't set socket to IPv6 only");
		exit(EXIT_FAILURE);
	}

	if (setsockopt(sock, IPPROTO_IPV6, IPV6_RECVPKTINFO, &one, sizeof(one))) {
		perror("can't set socket to deliver IPV6_PKTINFO control message");
		exit(EXIT_FAILURE);
	}

	server_addr.sin6_family = AF_INET6;
	server_addr.sin6_addr = in6addr_any;

	char *endptr;
	opterr = 0;

	int group_set = 0;
	uint64_t max_multicast_delay = 10000;

	int c;
	while ((c = getopt(argc, argv, "p:g:t:i:d:h")) != -1) {
		switch (c) {
		case 'p':
			server_addr.sin6_port = htons(atoi(optarg));
			break;

		case 'g':
			if (!inet_pton(AF_INET6, optarg, &mgroup_addr)) {
				perror("Invalid multicast group. This message will probably confuse you");
				exit(EXIT_FAILURE);
			}

			group_set = 1;
			break;

		case 'i':
			if (!group_set) {
				fprintf(stderr, "Multicast group must be given before interface.\n");
				exit(EXIT_FAILURE);
			}
			join_mcast(sock, mgroup_addr, optarg);
			break;

		case 't':
			max_multicast_delay = 1000 * strtoul(optarg, &endptr, 10);
			if (!*optarg || *endptr) {
				fprintf(stderr, "Invalid multicast delay\n");
				exit(EXIT_FAILURE);
			}
			break;

		case 'd':
			if (chdir(optarg)) {
				perror("Unable to change to given directory");
				exit(EXIT_FAILURE);
			}
			break;

		case 'h':
			usage();
			exit(EXIT_SUCCESS);
			break;

		default:
			fprintf(stderr, "Invalid parameter -%c ignored.\n", optopt);
		}
	}

	if (bind(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
		perror("bind failed");
		exit(EXIT_FAILURE);
	}

	load_providers();

	struct request_schedule schedule = {};

	while (true) {
		accept_request(&schedule, sock, max_multicast_delay);
		serve_request(&schedule, sock);
	}

	return EXIT_FAILURE;
}
