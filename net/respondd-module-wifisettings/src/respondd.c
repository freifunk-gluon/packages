#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include <limits.h>

#include <json-c/json.h>
#include <uci.h>

#include <respondd.h>

const unsigned int INVALID_CHANNEL = 0;
const unsigned int INVALID_TXPOWER = 0;

static inline unsigned char parse_option(const char *s, unsigned char invalid) {
	char *endptr = NULL;
	long int result;

	if (!s)
		return invalid;

	result = strtol(s, &endptr, 10);

	if (!endptr)
		return invalid;
	if ('\0' != *endptr)
		return invalid;
	if (result > UCHAR_MAX)
		return invalid;
	if (result < 0)
		return invalid;

	return (unsigned char)(result % UCHAR_MAX);
}

static struct json_object *respondd_provider_nodeinfo(void) {
	struct uci_context *ctx = NULL;
	struct uci_package *p = NULL;
	struct uci_section *s;
	struct uci_element *e;
	struct json_object *ret = NULL, *wireless = NULL, *v;
	unsigned char tmp;

	ctx = uci_alloc_context();
	if (!ctx)
		goto end;
	ctx->flags &= ~UCI_FLAG_STRICT;

	wireless = json_object_new_object();
	if (!wireless)
		goto end;

	ret = json_object_new_object();
	if (!ret)
		goto end;

	if (uci_load(ctx, "wireless", &p))
		goto end;

	uci_foreach_element(&p->sections, e) {
		s = uci_to_section(e);

		if(!strncmp(s->type,"wifi-device",11)){
			tmp = parse_option(uci_lookup_option_string(ctx, s, "channel"), INVALID_CHANNEL);
			if (tmp != INVALID_CHANNEL) {
				v = json_object_new_int64(tmp);
				if (!v)
					goto end;
				if (tmp >= 1 && tmp <= 14){
					json_object_object_add(wireless, "channel24", v);
					tmp = parse_option(uci_lookup_option_string(ctx, s, "txpower"), INVALID_TXPOWER);
					if (tmp != INVALID_TXPOWER) {
						v = json_object_new_int64(tmp);
						if (!v)
							goto end;
						json_object_object_add(wireless, "txpower24", v);
					}
				// FIXME lowes is 7, but i was able to differ between 2.4 Ghz and 5 Ghz by iwinfo_ops->frequency
				// In EU and US it is 36, so it would be okay for the moment (https://en.wikipedia.org/wiki/List_of_WLAN_channels)
				} else if (tmp >= 36 && tmp < 196){
					json_object_object_add(wireless, "channel5", v);
					tmp = parse_option(uci_lookup_option_string(ctx, s, "txpower"), INVALID_TXPOWER);
					if (tmp != INVALID_TXPOWER) {
						v = json_object_new_int64(tmp);
						if (!v)
							goto end;
						json_object_object_add(wireless, "txpower5", v);
					}
				} else
					json_object_object_add(wireless, "ErrorChannel", v);
			}
		}
	}

	json_object_object_add(ret, "wireless", wireless);
end:
	if (ctx) {
		if (p)
			uci_unload(ctx, p);
			uci_free_context(ctx);
	}
	return ret;

}

const struct respondd_provider_info respondd_providers[] = {
	{"nodeinfo", respondd_provider_nodeinfo},
	{0,0},
};
