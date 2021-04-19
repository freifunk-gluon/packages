#include <string.h>
#include <stdio.h>
#include <json-c/json.h>
#include <respondd.h>

#include "airtime.h"
#include "ifaces.h"

static struct json_object *respondd_provider_statistics(void) {
	bool ok;
	int newest_element_index;
	struct json_object *last, *result, *wireless;
	struct iface_list *ifaces;

	result = json_object_new_object();
	if (!result)
		return NULL;

	wireless = json_object_new_array();
	if (!wireless) {
		json_object_put(result);
		return NULL;
	}

	ifaces = get_ifaces();
	while (ifaces != NULL) {
		ok = get_airtime(wireless, ifaces->ifx);
		if (ok) {
			newest_element_index = json_object_array_length(wireless)-1;
			last = json_object_array_get_idx(wireless, newest_element_index);
			json_object_object_add(last, "phy", json_object_new_int(ifaces->wiphy));
		}
		void *freeptr = ifaces;
		ifaces = ifaces->next;
		free(freeptr);
	}

	json_object_object_add(result, "wireless", wireless);
	return result;
}

const struct respondd_provider_info respondd_providers[] = {
	{"statistics", respondd_provider_statistics},
	{0, 0},
};
