// SPDX-License-Identifier: BSD-2-Clause
// SPDX-FileCopyrightText: 2016 Julian Kornberger <jk+freifunk@digineo.de>
// SPDX-FileCopyrightText: 2016 Martin Müller <geno+ffhb@fireorbit.de>
// SPDX-FileCopyrightText: 2016 Jan-Philipp Litza <janphilipp@litza.de>

#include <linux/nl80211.h>
#include <netlink/genl/genl.h>

#include "netlink.h"
#include "airtime.h"

/*
 * Excerpt from nl80211.h:
 * enum nl80211_survey_info - survey information
 *
 * These attribute types are used with %NL80211_ATTR_SURVEY_INFO
 * when getting information about a survey.
 *
 * @__NL80211_SURVEY_INFO_INVALID: attribute number 0 is reserved
 * @NL80211_SURVEY_INFO_FREQUENCY: center frequency of channel
 * @NL80211_SURVEY_INFO_NOISE: noise level of channel (u8, dBm)
 * @NL80211_SURVEY_INFO_IN_USE: channel is currently being used
 * @NL80211_SURVEY_INFO_CHANNEL_TIME: amount of time (in ms) that the radio
 *	spent on this channel
 * @NL80211_SURVEY_INFO_CHANNEL_TIME_BUSY: amount of the time the primary
 *	channel was sensed busy (either due to activity or energy detect)
 * @NL80211_SURVEY_INFO_CHANNEL_TIME_EXT_BUSY: amount of time the extension
 *	channel was sensed busy
 * @NL80211_SURVEY_INFO_CHANNEL_TIME_RX: amount of time the radio spent
 *	receiving data
 * @NL80211_SURVEY_INFO_CHANNEL_TIME_TX: amount of time the radio spent
 *	transmitting data
 * @NL80211_SURVEY_INFO_MAX: highest survey info attribute number
 *	currently defined
 * @__NL80211_SURVEY_INFO_AFTER_LAST: internal use
 */

static const char *const msg_names[NL80211_SURVEY_INFO_MAX + 1] = {
	[NL80211_SURVEY_INFO_FREQUENCY] = "frequency",
	[NL80211_SURVEY_INFO_CHANNEL_TIME] = "active",
	[NL80211_SURVEY_INFO_CHANNEL_TIME_BUSY] = "busy",
	[NL80211_SURVEY_INFO_CHANNEL_TIME_RX] = "rx",
	[NL80211_SURVEY_INFO_CHANNEL_TIME_TX] = "tx",
	[NL80211_SURVEY_INFO_NOISE] = "noise",
};

static int survey_airtime_handler(struct nl_msg *msg, void *arg) {
	struct json_object **result = arg;
	struct json_object *freq_json = NULL;

	struct genlmsghdr *gnlh = nlmsg_data(nlmsg_hdr(msg));
	struct nlattr *survey_info = nla_find(genlmsg_attrdata(gnlh, 0), genlmsg_attrlen(gnlh, 0), NL80211_ATTR_SURVEY_INFO);

	if (*result) {
		fprintf(stderr, "respondd-module-airtime: callback called again after NL_STOP\n");
		return NL_STOP;
	}

	if (!survey_info) {
		fprintf(stderr, "respondd-module-airtime: survey data missing in netlink message\n");
		return NL_STOP;
	}

	freq_json = json_object_new_object();
	if (!freq_json) {
		fprintf(stderr, "respondd-module-airtime: failed allocating JSON object\n");
		return NL_STOP;
	}

	// This variable counts the number of required attributes that are
	// found in the message and is afterwards checked against the number of
	// required attributes.
	unsigned int req_fields = 0;

	int rem;
	struct nlattr *nla;
	nla_for_each_nested(nla, survey_info, rem) {
		int type = nla_type(nla);

		if (type > NL80211_SURVEY_INFO_MAX)
			continue;

		switch (type) {
			// these are the required fields
			case NL80211_SURVEY_INFO_IN_USE:
			case NL80211_SURVEY_INFO_FREQUENCY:
			case NL80211_SURVEY_INFO_CHANNEL_TIME:
				req_fields++;
		}

		if (!msg_names[type])
			continue;

		struct json_object *data_json = NULL;
		switch (nla_len(nla)) {
			case sizeof(uint64_t):
				data_json = json_object_new_int64(nla_get_u64(nla));
				break;
			case sizeof(uint32_t):
				data_json = json_object_new_int(nla_get_u32(nla));
				break;
			case sizeof(uint8_t):
				if (type == NL80211_SURVEY_INFO_NOISE)
					data_json = json_object_new_int((int8_t)nla_get_u8(nla));
				else
					data_json = json_object_new_int(nla_get_u8(nla));
				break;
			default:
				fprintf(stderr, "respondd-module-airtime: Unexpected NL attribute length: %d\n", nla_len(nla));
		}

		if (data_json)
			json_object_object_add(freq_json, msg_names[type], data_json);
	}

	/* The kernel didn't report all required fields: continue with next
	 * record, this entry was off-channel */
	if (req_fields != 3) {
		json_object_put(freq_json);
		return NL_OK;
	}

	*result = freq_json;
	return NL_STOP;
}

struct json_object * get_airtime(int ifx) {
	struct json_object *result = NULL;
	nl_send_dump(survey_airtime_handler, &result, NL80211_CMD_GET_SURVEY, ifx);
	return result;
}
