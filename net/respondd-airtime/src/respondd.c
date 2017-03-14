#include <string.h>
#include <stdio.h>
#include <json-c/json.h>
#include <respondd.h>

#include "airtime.h"

#if GLUON
static const char const *wifi_0_dev = "client0";
static const char const *wifi_1_dev = "client1";

#else
static const char const *wifi_0_dev = "wlan0";
static const char const *wifi_1_dev = "wlan1";

#endif /* GLUON */


void fill_airtime_json(struct airtime_result *air, struct json_object* wireless){
	struct json_object *ret = NULL, *obj = NULL;

	obj = json_object_new_object();
	if(!obj)
		goto error;
#define JSON_ADD_INT64(value,key) {ret = json_object_new_int64(value); json_object_object_add(obj,key,ret);}
	ret = json_object_new_int(air->frequency);
	if(!ret)
		goto error;
	json_object_object_add(obj,"frequency",ret);

	JSON_ADD_INT64(air->active_time.current,"active")
	JSON_ADD_INT64(air->busy_time.current,"busy")
	JSON_ADD_INT64(air->rx_time.current,"rx")
	JSON_ADD_INT64(air->tx_time.current,"tx")

	ret = json_object_new_int(air->noise);
	json_object_object_add(obj,"noise",ret);

error:
	if(air->frequency >= 2400  && air->frequency < 2500)
		json_object_object_add(wireless, "airtime24", obj);
	else if (air->frequency >= 5000 && air->frequency < 6000)
		json_object_object_add(wireless, "airtime5", obj);
}

static struct json_object *respondd_provider_statistics(void) {
	struct airtime *a = NULL;
	struct json_object *ret = NULL, *wireless = NULL;

	wireless = json_object_new_object();
	if (!wireless)
		return NULL;

	ret = json_object_new_object();
	if (!ret)
		return NULL;

	a = get_airtime(wifi_0_dev,wifi_1_dev);
	if (!a)
		goto end;

	if (a->radio0.frequency)
		fill_airtime_json(&a->radio0,wireless);

	if (a->radio1.frequency)
		fill_airtime_json(&a->radio1,wireless);

end:
	json_object_object_add(ret, "wireless", wireless);
	return ret;
}

const struct respondd_provider_info respondd_providers[] = {
	{"statistics", respondd_provider_statistics},
	{0, 0},
};
