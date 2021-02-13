/*
   Copyright (c) 2014-2015, Nils Schneider <nils@nilsschneider.net>
   Copyright (c) 2015-2016, Matthias Schiffer <mschiffer@universe-factory.net>
   Copyright (c) 2016 Leonardo Mörlein <me@irrelefant.net>
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
#include <fcntl.h>
#include <inttypes.h>
#include <search.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <time.h>
#include <unistd.h>
#include <errno.h>

#include <arpa/inet.h>
#include <net/if.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>

#include <ecdsautil/ecdsa.h>
#include <ecdsautil/sha256.h>
#include <uci.h>

#define SCHEDULE_LEN 8
#define REQUEST_MAXLEN 256
#define MAX_MULTICAST_DELAY_DEFAULT 0

ecc_int256_t ed25519_secret;
ecc_int256_t ed25519_public;

struct interface_info {
	struct interface_info *next;

	unsigned int ifindex;
	uint64_t max_multicast_delay;
};

struct group_info {
	struct group_info *next;
	struct in6_addr address;

	struct interface_info *interfaces;
};

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

struct request_task {
	struct request_task *next;
	int64_t scheduled_time;

	struct sockaddr_in6 client_addr;
	char request[REQUEST_MAXLEN];
};

struct request_schedule {
	size_t length;
	struct request_task *list_head;
};

static int64_t now;
static struct hsearch_data htab;


static struct json_object * merge_json(struct json_object *a, struct json_object *b);


static void usage() {
	puts("Usage:");
	puts("  respondd -h");
	puts("  respondd [-p <port>] [-g <group> -i <if0> [-i <if1> ..]] [-d <dir> [-d <dir> ..]]");
	puts("        -p <int>         port number to listen on");
	puts("        -g <ip6>         multicast group, e.g. ff02::2:1001");
	puts("        -i <string>      interface on which the group is joined");
	puts("        -t <int>         maximum delay seconds before multicast responses");
	puts("                         for the last specified multicast interface (default: 0)");
	puts("        -d <string>      data provider directory");
	puts("        -h               this help\n");
}

// returns true on success
static bool join_mcast(const int sock, const struct in6_addr addr, unsigned int ifindex) {
	struct ipv6_mreq mreq;

	mreq.ipv6mr_multiaddr = addr;
	mreq.ipv6mr_interface = ifindex;

	if (mreq.ipv6mr_interface == 0) {
		fprintf(stderr, "join_mcast: no valid interface given\n");
		return false;
	}

	if (setsockopt(sock, IPPROTO_IPV6, IPV6_JOIN_GROUP, &mreq, sizeof(mreq)) == -1) {
		perror("setsockopt: unable to join multicast group");
		return false;
	}

	return true;
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
	if (!handle) {
		syslog(LOG_WARNING, "unable to open provider module '%s', ignoring: %s", filename, dlerror());
		return NULL;
	}

	// clean a potential previous error
	dlerror();

	const struct respondd_provider_info *ret = dlsym(handle, "respondd_providers");
	if (!ret) {
		syslog(LOG_WARNING,
				"unable to load providers from '%s', ignoring: %s",
				filename, dlerror() ?: "'respondd_providers' == NULL");
		dlclose(handle);
		return NULL;
	}

	return ret;
}

static bool schedule_push_request(struct request_schedule *s, struct request_task *new_task) {
	if (s->length >= SCHEDULE_LEN)
		// schedule is full
		return false;

	// insert into sorted list
	struct request_task **pos;
	for (pos = &s->list_head; *pos; pos = &((*pos)->next)) {
		if ((*pos)->scheduled_time > new_task->scheduled_time)
			break;
	}
	// insert before *pos
	new_task->next = *pos;
	*pos = new_task;

	s->length++;
	return true;
}

static int64_t schedule_idle_time(struct request_schedule *s) {
	if (!s->list_head)
		// nothing to do yet (0 = infinite time)
		return 0;

	int64_t result = s->list_head->scheduled_time - now;

	if (result <= 0)
		return -1; // zero is infinity
	else
		return result;
}

static struct request_task * schedule_pop_request(struct request_schedule *s) {
	if (!s->list_head)
		// schedule is empty
		return NULL;

	if (schedule_idle_time(s) >= 0) {
		// nothing to do yet
		return NULL;
	}

	struct request_task *result = s->list_head;
	s->list_head = s->list_head->next;
	s->length--;

	return result;
}

static void load_cache_time(struct request_type *r, const char *name) {
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
		struct request_type *r = calloc(1, sizeof(*r));
		r->cache_timeout = now;

		key.data = r;
		if (!hsearch_r(key, ENTER, &entry, &htab)) {
			perror("hsearch_r");
			exit(EXIT_FAILURE);
		}
	}

	struct request_type *r = entry->data;
	load_cache_time(r, provider->request);

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

static void load_providers(const char *path) {
	update_time();

	int cwdfd = open(".", O_DIRECTORY);

	if (chdir(path)) {
		syslog(LOG_INFO, "unable to read providers from '%s', ignoring", path);
		goto out;
	}

	DIR *dir = opendir(".");
	if (!dir) {
		perror("opendir");
		goto out;
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

out:
	fchdir(cwdfd);
	close(cwdfd);
}

static struct json_object * eval_providers(struct provider_list *providers) {
	struct json_object *ret = json_object_new_object();

	for (; providers; providers = providers->next)
		ret = merge_json(providers->provider(), ret);

	return ret;
}

int random_bytes(unsigned char *buffer, size_t len) {
	int fd;
	size_t read_bytes = 0;

	fd = open("/dev/random", O_RDONLY);

	if (fd < 0) {
		fprintf(stderr, "Can't open /dev/random: %s\n", strerror(errno));
		goto out_error;
	}

	while (read_bytes < len) {
		ssize_t ret = read(fd, buffer + read_bytes, len - read_bytes);

		if (ret < 0) {
			if (errno == EINTR)
				continue;

			fprintf(stderr, "Unable to read random bytes: %s\n", strerror(errno));
			goto out_error;
		}

		read_bytes += ret;
	}

	close(fd);
	return 1;

out_error:
	close(fd);
	return 0;
}

// str must be a char[2*(offset+len)+1]
static void sprintf_hex(char *str_buf, const uint8_t *buf, size_t len, size_t offset) {
	str_buf += 2*offset;
	for (size_t i = 0; i < len; i++) {
		snprintf(str_buf, 3, "%02hhx", buf[i]);
		str_buf += 2;
	}
}

int parsehex(void *buffer, const char *string, size_t len) {
	// number of digits must be even
	if ((strlen(string) & 1) == 1)
		return 0;

	// number of digits must be 2 * len
	if (strlen(string) != 2 * len)
		return 0;

	while (len--) {
		int ret;
		ret = sscanf(string, "%02hhx", (char*)(buffer++));
		string += 2;

		if (ret != 1)
			break;
	}

	if (len != -1)
		return 0;

	return 1;
}

ecc_int256_t read_or_generate_key() {
	struct uci_context *ctx = uci_alloc_context();
	if (!ctx) {
		fprintf(stderr, "respondd: error: failed to allocate UCI context\n");
		abort();
	}

	ctx->flags &= ~UCI_FLAG_STRICT;

	struct uci_package *p;
	struct uci_section *s;

	if (uci_load(ctx, "respondd", &p) != UCI_OK) {
		fputs("respondd: error: unable to load UCI package\n", stderr);
		exit(1);
	}

	s = uci_lookup_section(ctx, p, "settings");
	if (!s || strcmp(s->type, "respondd")) {
		fputs("respondd: error: could not load UCI section respondd.settings\n", stderr);
		exit(1);
	}

	const char *secret_str = uci_lookup_option_string(ctx, s, "secret");
	ecc_int256_t secret;

	if (!secret_str || !parsehex(&secret, secret_str, 32)) {
		fputs("respondd: no valid key found. generating new key.\n", stderr);

		// generate it
		if (!random_bytes(secret.p, 32)) {
			fputs("respondd: unable to read random bytes.\n", stderr);
			exit(1);
		}
		ecc_25519_gf_sanitize_secret(&secret, &secret);

		// save it to uci
		char secret_str_new[64+1];
		sprintf_hex(secret_str_new, secret.p, 32, 0);
		struct uci_ptr ptr ={
			.package = "respondd",
			.section = "settings",
			.option = "secret",
			.value = secret_str_new,
		};
		uci_set(ctx, &ptr);
		uci_commit(ctx, &ptr.p, false);
		uci_unload(ctx, ptr.p);
		fputs("respondd: key generated and saved.\n", stderr);
	}

	uci_free_context(ctx);

	return secret;
}

static void public_from_secret(ecc_int256_t *pub, const ecc_int256_t *secret) {
	ecc_25519_work_t work;
	ecc_25519_scalarmult_base(&work, secret);
	ecc_25519_store_packed_legacy(pub, &work);
}

// The string representation of obj is signed using the secret. After signing,
// a structure containing the public key and the signature is added into obj.
// The obj looks like this after calling sign_json(obj, ...):
//
//    {
//        ...,
//        "auth": {
//            "pub": "25077b1914533e94a60853678b8484531a5f63463de87786f042e3d88d0bbc27",
//            "sig": "eca0455a99a6b79edc719c18aa46c7d8f960e041f77f836326e6eae08064606320daff6f11cb0d0a2fb51a346725e3dc01e9a85f7c064ec857200c302937409"
//        }
//    }
//
// To verify the signature, the substructure "auth" has to be removed before.
// The string representation of obj has to be densely packed. No whitespace and
// tabs " " between keys, no newlines and the order of keys must not be changed.
static void sign_json(struct json_object * obj, const ecc_int256_t *secret, const ecc_int256_t *pub) {
	// TODO: This currently enables replay attacks, as the json does not contain
	//       any time value or so... However, we do not care much, as
	//       this is probably not an effective attack vector.
	const char *str = json_object_to_json_string_ext(obj, JSON_C_TO_STRING_PLAIN);

	// hash
	ecc_int256_t hash;
	ecdsa_sha256_context_t hash_ctx;
	ecdsa_sha256_init(&hash_ctx);
	ecdsa_sha256_update(&hash_ctx, str, strlen(str));
	ecdsa_sha256_final(&hash_ctx, hash.p);

	struct json_object *auth = json_object_new_object();

	// generate signature
	ecdsa_signature_t signature;
	char signature_str[128+1];
	ecdsa_sign_legacy(&signature, &hash, secret);
	sprintf_hex(signature_str, signature.r.p, 32, 0);
	sprintf_hex(signature_str, signature.s.p, 32, 32);
	json_object_object_add(auth, "signature", json_object_new_string(signature_str));

	// append pubkey
	char pub_str[64+1];
	sprintf_hex(pub_str, pub->p, 32, 0);
	json_object_object_add(auth, "pubkey", json_object_new_string(pub_str));

	json_object_object_add(obj, "auth", auth);
}

/**
 * Find all providers for the type and return the (eventually cached) result
 *
 * Either the request can be answered from cache or eval_providers() is called
 * to get fresh results.
 *
 * @type: String containing the query type
 *
 * Returns: Result for the query as json object
 */
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

	sign_json(ret, &ed25519_secret, &ed25519_public);

	if (r->cache_time) {
		if (r->cache)
			json_object_put(r->cache);

		r->cache = json_object_get(ret);
		r->cache_timeout = now + r->cache_time;
	}

	return ret;
}

/**
 * Calls single_request() for each query type and merges the results
 *
 * @types: String with space seperated list of types. E.g. "type1 type2"
 *
 * Returns: The json structure is { "type1": {...}, "type2": {...} }
 */
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

/**
 * Calls multi_request() or single_request() depending on the request type
 *
 * @request: Request string. Two patterns are possible:
 *           - "type" (single request)
 *           - "GET type1 type2 ..." (multi request)
 * @compress: Responses to multi requests should be compressed afterwards by
 *            the calling function, this will be saved in *compress.
 *
 * Returns: The uncompressed json result ready to be (compressed and) sent
 */
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

/**
 * Eventually compress and send response the response on the udp socket
 *
 * @sock: Socket filedescriptor of the udp socket
 * @result: Result json object to be send
 * @compress: True, if the answer should be compressed before sending
 * @addr: Ipv6 destination address for the answer
 */
static void send_response(int sock, struct json_object *result, bool compress,
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
		if (sendto(sock, output, output_bytes, 0, (struct sockaddr *) addr, sizeof(*addr)) < 0)
			perror("sendto failed");
	}

	json_object_put(result);
}

/**
 * Handle the request task and the send response
 *
 * Calls handle_request() and if successful send_response() afterwards.
 *
 * @task: The task object (including the request query and the response address)
 *        for the task.
 */
static void serve_request(struct request_task *task, int sock) {
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
}

static const struct interface_info * find_multicast_interface(const struct group_info *groups, unsigned ifindex, const struct in6_addr *addr) {
	for (const struct group_info *group = groups; group; group = group->next) {
		if (memcmp(addr, &group->address, sizeof(struct in6_addr)) != 0)
			continue;

		for (const struct interface_info *iface = group->interfaces; iface; iface = iface->next) {
			if (ifindex != iface->ifindex)
				continue;

			return iface;
		}
	}

	return NULL;
}

/**
 * Wait for an incoming request and schedule it.
 *
 * 1a. If the schedule is empty, we wait infinite time.
 * 1b. If we have scheduled requests, we only wait for incoming requests
 *     until we reach the scheduling deadline.
 * 1c. If there is no request incomming in the above time, the fuction will
 *     return.
 * 2a. If the incoming request was sent to a multicast destination IPv6,
 *     check whether there was set a max multicast delay for the incomming iface
 *     in if_delay_info_list.
 * 2b. If so choose a random delay between 0 and max_multicast_delay milliseconds
 *     and schedule the request.
 * 2c. If not, send the request immediately.
 * 2d. If the schedule is full, send the reply immediately.
 * 3a. If the incoming request was sent to a unicast destination, the response
 *     will be also sent immediately.
 */
static void accept_request(struct request_schedule *schedule, int sock,
                           const struct group_info *groups) {
	char input[REQUEST_MAXLEN];
	ssize_t input_bytes;
	struct sockaddr_in6 addr;
	char control[256];
	struct in6_addr destaddr = {};
	struct cmsghdr *cmsg;
	unsigned int ifindex = 0;
	int recv_errno;

	int64_t timeout = schedule_idle_time(schedule);
	if (timeout < 0)
		return;

	// set timeout to the socket
	struct timeval t;
	t.tv_sec = ((uint64_t) timeout) / 1000;
	t.tv_usec = (((uint64_t) timeout) % 1000) * 1000;

	if (setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &t, sizeof(t)) < 0)
		perror("setsockopt failed");

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
	recv_errno = errno;
	update_time();

	// Timeout
	errno = recv_errno;
	if (input_bytes < 0 && (errno == EAGAIN || errno == EWOULDBLOCK))
		return;

	if (input_bytes < 0) {
		perror("recvmsg failed");
		exit(EXIT_FAILURE);
	}

	// determine destination address
	for (cmsg = CMSG_FIRSTHDR(&mh);	cmsg != NULL; cmsg = CMSG_NXTHDR(&mh, cmsg))
	{
		// skip other packet headers
		if (cmsg->cmsg_level != IPPROTO_IPV6 || cmsg->cmsg_type != IPV6_PKTINFO)
			continue;

		struct in6_pktinfo *pi = (struct in6_pktinfo *) CMSG_DATA(cmsg);
		destaddr = pi->ipi6_addr;
		ifindex = pi->ipi6_ifindex;
		break;
	}

	input[input_bytes] = 0;

	const struct interface_info *iface = NULL;
	if (IN6_IS_ADDR_MULTICAST(&destaddr)) {
		iface = find_multicast_interface(groups, ifindex, &destaddr);
		// this should not happen
		if (!iface)
			return;
	}

	struct request_task *new_task = malloc(sizeof(*new_task));
	// input_bytes cannot be greater than REQUEST_MAXLEN-1
	memcpy(new_task->request, input, input_bytes + 1);
	new_task->scheduled_time = 0;
	new_task->client_addr = addr;

	bool is_scheduled;
	if (iface && iface->max_multicast_delay) {
		// scheduling could fail because the schedule is full
		new_task->scheduled_time = now + rand() % iface->max_multicast_delay;
		is_scheduled = schedule_push_request(schedule, new_task);
	} else {
		// unicast packets are always sent directly
		is_scheduled = false;
	}

	if (!is_scheduled) {
		// reply immediately
		serve_request(new_task, sock);
		free(new_task);
	}
}

int main(int argc, char **argv) {
	const int one = 1;

	int sock;
	struct sockaddr_in6 server_addr = {};
	struct in6_addr mgroup_addr;

	srand(time(NULL));

	/* Maximum number of request types, might be made configurable in the future */
	if (!hcreate_r(32, &htab)) {
		perror("hcreate_r");
		exit(EXIT_FAILURE);
	}

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

	struct group_info *groups = NULL;

	openlog("respondd", LOG_PID, LOG_DAEMON);

	int c;
	while ((c = getopt(argc, argv, "p:g:t:i:d:h")) != -1) {
		switch (c) {
		case 'p':
			server_addr.sin6_port = htons(atoi(optarg));
			break;

		case 'g':
			if (!inet_pton(AF_INET6, optarg, &mgroup_addr)) {
				fprintf(stderr, "Invalid multicast group address.\n");
				exit(EXIT_FAILURE);
			}

			struct group_info *new_group = malloc(sizeof(*new_group));
			new_group->address = mgroup_addr;
			new_group->interfaces = NULL;
			new_group->next = groups;
			groups = new_group;
			break;

		case 'i':
			if (!groups) {
				fprintf(stderr, "Multicast group must be given before interface.\n");
				exit(EXIT_FAILURE);
			}
			int ifindex = if_nametoindex(optarg);
			if (!join_mcast(sock, mgroup_addr, ifindex)) {
				fprintf(stderr, "Could not join multicast group on %s: ", optarg);
				continue;
			}

			struct interface_info *new_iface = malloc(sizeof(*new_iface));
			new_iface->ifindex = ifindex;
			new_iface->max_multicast_delay = MAX_MULTICAST_DELAY_DEFAULT;
			new_iface->next = groups->interfaces;
			groups->interfaces = new_iface;

			break;

		case 't':
			if (!groups || !groups->interfaces) {
				fprintf(stderr, "Interface must be given before max response delay.\n");
				exit(EXIT_FAILURE);
			}

			uint64_t max_multicast_delay = UINT64_C(1000) * strtoul(optarg, &endptr, 10);
			if (!*optarg || *endptr || max_multicast_delay > INT64_MAX) {
				fprintf(stderr, "Invalid multicast delay\n");
				exit(EXIT_FAILURE);
			}

			groups->interfaces->max_multicast_delay = max_multicast_delay;

			break;

		case 'd':
			load_providers(optarg);
			break;

		case 'h':
			usage();
			exit(EXIT_SUCCESS);
			break;

		default:
			fprintf(stderr, "Invalid parameter -%c.\n", optopt);
			exit(EXIT_FAILURE);
		}
	}

	// load keys for ed25519 signatures
	ed25519_secret = read_or_generate_key();
	public_from_secret(&ed25519_public, &ed25519_secret);

	if (bind(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
		perror("bind failed");
		exit(EXIT_FAILURE);
	}

	struct request_schedule schedule = {};

	while (true) {
		accept_request(&schedule, sock, groups);

		struct request_task *task = schedule_pop_request(&schedule);

		if (!task)
			continue;

		serve_request(task, sock);
		free(task);
	}

	return EXIT_FAILURE;
}
