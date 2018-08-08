#include <string.h>
#include <stdio.h>
#include <json-c/json.h>
#include <respondd.h>

#include "airtime.h"
#include "clients.h"
#include "ifaces.h"

static struct json_object *respondd_provider_statistics(void) {
	struct json_object *result, *wireless, *clients, *interface;
	struct iface_list *ifaces;
	int wifi24 = 0, wifi5 = 0, clients_count;

	result = json_object_new_object();
	if (!result)
		return NULL;

	wireless = json_object_new_array();
	if (!wireless) {
		//TODO why needed? : json_object_put(result);
		return NULL;
	}

	clients = json_object_new_object();
	if (!clients) {
		return NULL;
	}

	ifaces = get_ifaces();
	while (ifaces != NULL) {
		clients_count = 0;
		get_client_counts(&clients_count, ifaces->ifx);
		if (ifaces->frequency < 5000)
			wifi24 += clients_count;
		if (ifaces->frequency > 5000)
			wifi5 += clients_count;

		//TODO wiphy only one radio added? (no necessary on gluon - only one ap-ssid at radio)
		interface = json_object_new_object();
		if (!interface) {
			continue;
		}
		json_object_object_add(interface, "frequency", json_object_new_int(ifaces->frequency));
		json_object_object_add(interface, "txpower", json_object_new_int(ifaces->txpower));
		get_airtime(interface, ifaces->ifx);
		//TODO remove at merge radios (one wiphy radio)
		json_object_object_add(interface, "clients", json_object_new_int(clients_count));
		json_object_array_add(wireless, interface);

		void *freeptr = ifaces;
		ifaces = ifaces->next;
		free(freeptr);
	}

	//TODO maybe skip: if (wifi24 >  0 || wifi5 > 0) {
	json_object_object_add(clients, "wifi24", json_object_new_int(wifi24));
	json_object_object_add(clients, "wifi5", json_object_new_int(wifi5));
	json_object_object_add(result, "clients", clients);
	//}

	json_object_object_add(result, "wireless", wireless);
	return result;
}

const struct respondd_provider_info respondd_providers[] = {
	{"statistics", respondd_provider_statistics},
	{0, 0},
};
