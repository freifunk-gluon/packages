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
#include <errno.h>
#include <error.h>
#include <inttypes.h>
#include <search.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include <arpa/inet.h>
#include <net/if.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>


struct provider_list {
	struct provider_list *next;

	char *name;
	respondd_provider provider;
};

struct request_type {
	struct provider_list *providers;

	struct json_object *cache;
	uint64_t cache_time;
	int64_t cache_timeout;
};

struct ifidx_list {
	struct ifidx_list *next;
	int idx;
};


static int sock;
static int64_t now;
static struct in6_addr mgroup_addr = IN6ADDR_ANY_INIT;
static struct hsearch_data htab;
static struct ifidx_list *ifaces = NULL;
static char *iface_list_path = NULL;


static struct json_object * merge_json(struct json_object *a, struct json_object *b);


static void usage() {
	puts("Usage:");
	puts("  respondd -h");
	puts("  respondd [-p <port>] [-g <group> -c <iface_list_file>] [-d <data_dir>]");
	puts("        -p <int>         port number to listen on");
	puts("        -g <ip6>         multicast group, e.g. ff02::2:1001");
	puts("        -c <string>      file with one iface name per line, on all of which");
	puts("                         the multicast group is joined");
	puts("        -d <string>      data provider directory (default: current directory)");
	puts("        -h               this help\n");
	puts("The <iface_list_file> is reloaded on SIGHUP.\n");
}

static void mcast_membership(int iface, bool member) {
	struct ipv6_mreq mreq;

	mreq.ipv6mr_multiaddr = mgroup_addr;
	mreq.ipv6mr_interface = iface;

	if (setsockopt(sock, IPPROTO_IPV6, member? IPV6_ADD_MEMBERSHIP : IPV6_DROP_MEMBERSHIP, &mreq, sizeof(mreq)) == -1)
		goto error;

	return;

 error:
	error(0, errno, "Could not %s multicast group on interface #%d",
		member? "join" : "leave",
		iface);
}


static void update_time(void) {
	struct timespec tp;
	clock_gettime(CLOCK_MONOTONIC, &tp);

	now = (int64_t)tp.tv_sec * 1000 + tp.tv_nsec / 1000000;
}


static void read_iface_list() {
	FILE *f;
	struct ifidx_list *tmp_item, *iter_new, *iter_old = ifaces;
	struct ifidx_list **prev_ptr;
	char *line = NULL;
	size_t size = 0;
	int len;

	if (IN6_IS_ADDR_UNSPECIFIED(&mgroup_addr) || iface_list_path == NULL)
		return;

	f = fopen(iface_list_path, "r");
	if (f == NULL) {
		error(0, errno, "warning: could not open iface list %s", iface_list_path);
		return;
	}
	ifaces = NULL;
	while ((len = getline(&line, &size, f)) != -1) {
		if (line[len-1] == '\n')
			line[len-1] = '\0';
		tmp_item = malloc(sizeof(struct ifidx_list));
		tmp_item->idx = if_nametoindex(line);
		if (tmp_item->idx == 0) {
			error(0, errno, "warning: could not find index for interface %s", line);
			free(tmp_item);
			continue;
		}

		// Insert into sorted list ifaces
		prev_ptr = &ifaces;
		for (iter_new = ifaces; iter_new != NULL; iter_new = iter_new->next) {
			if (iter_new->idx > tmp_item->idx)
				break;
			prev_ptr = &iter_new->next;
		}
		tmp_item->next = iter_new;
		*prev_ptr = tmp_item;
	}
	fclose(f);

	for (iter_new = ifaces; iter_old != NULL || iter_new != NULL; ) {
		if (iter_new == NULL || (iter_old != NULL && iter_old->idx < iter_new->idx)) {
			// interface iter_old->idx disappeared
			mcast_membership(iter_old->idx, false);
			tmp_item = iter_old;
			iter_old = iter_old->next;
			free(tmp_item);
		}
		else if (iter_old == NULL || iter_old->idx > iter_new->idx) {
			// interface iter_new->idx was added
			mcast_membership(iter_new->idx, true);
			iter_new = iter_new->next;
		}
		else if (iter_old->idx == iter_new->idx) {
			// interface didn't change
			iter_new = iter_new->next;
			tmp_item = iter_old;
			iter_old = iter_old->next;
			free(tmp_item);
		}
	}
}

static void signal_handler(int signal) {
	if (signal == SIGHUP)
		read_iface_list();
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

	update_time();

	if (!strncmp(request, "GET ", 4)) {
		*compress = true;
		return multi_request(request+4);
	}
	else {
		*compress = false;
		return single_request(request);
	}
}


static void serve(int sock) {
	char input[256];
	const char *output = NULL;
	ssize_t input_bytes, output_bytes;
	struct sockaddr_in6 addr;
	socklen_t addrlen = sizeof(addr);
	bool compress;

	input_bytes = recvfrom(sock, input, sizeof(input)-1, 0, (struct sockaddr *)&addr, &addrlen);

	if (input_bytes == EINTR)
		return;

	if (input_bytes < 0) {
		perror("recvfrom failed");
		exit(EXIT_FAILURE);
	}

	input[input_bytes] = 0;

	struct json_object *result = handle_request(input, &compress);
	if (!result)
		return;

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
		if (sendto(sock, output, output_bytes, 0, (struct sockaddr *)&addr, addrlen) < 0)
			perror("sendto failed");
	}

	json_object_put(result);
}


int main(int argc, char **argv) {
	const int one = 1;

	struct sigaction sa = {
		.sa_handler = &signal_handler,
		.sa_flags = SA_RESTART,
	};

	struct sockaddr_in6 server_addr = {};

	sock = socket(PF_INET6, SOCK_DGRAM, 0);

	if (sock < 0) {
		perror("creating socket");
		exit(EXIT_FAILURE);
	}

	if (setsockopt(sock, IPPROTO_IPV6, IPV6_V6ONLY, &one, sizeof(one))) {
		perror("can't set socket to IPv6 only");
		exit(EXIT_FAILURE);
	}

	server_addr.sin6_family = AF_INET6;
	server_addr.sin6_addr = in6addr_any;

	opterr = 0;

	int c;
	while ((c = getopt(argc, argv, "p:g:c:d:h")) != -1) {
		switch (c) {
		case 'p':
			server_addr.sin6_port = htons(atoi(optarg));
			break;

		case 'g':
			if (!inet_pton(AF_INET6, optarg, &mgroup_addr) ||
					!IN6_IS_ADDR_MULTICAST(&mgroup_addr)) {
				perror("Invalid multicast group. This message will probably confuse you");
				exit(EXIT_FAILURE);
			}
			break;

		case 'c':
			iface_list_path = optarg;
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

	if ((iface_list_path == NULL) != (IN6_IS_ADDR_UNSPECIFIED(&mgroup_addr))) {
		fprintf(stderr, "Error: only one of -g and -c given!\n");
		usage();
		exit(EXIT_FAILURE);
	}

	if (bind(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
		perror("bind failed");
		exit(EXIT_FAILURE);
	}

	load_providers();
	read_iface_list();

	sigaction(SIGHUP, &sa, NULL);

	while (true)
		serve(sock);

	return EXIT_FAILURE;
}
