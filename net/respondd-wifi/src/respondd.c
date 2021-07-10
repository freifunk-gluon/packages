#include <string.h>
#include <stdio.h>
#include <json-c/json.h>
#include <respondd.h>

#include <linux/nl80211.h>

#include "clients.h"
#include "neighbours.h"
#include "ifaces.h"

static struct json_object *respondd_provider_statistics(void) {
	struct json_object *result, *clients;
	struct iface_list *ifaces;
	int wifi24 = 0, wifi5 = 0, clients_count;

	result = json_object_new_object();
	if (!result)
		return NULL;

	clients = json_object_new_object();
	if (!clients) {
		json_object_put(wireless);
		json_object_put(result);
		return NULL;
	}

	ifaces = get_ifaces();
	while (ifaces != NULL) {
		if(ifaces->type != NL80211_IFTYPE_AP)
			goto next_statistics;
		
		clients_count = 0;
		get_client_counts(&clients_count, ifaces->ifx);
		if (ifaces->frequency < 5000)
			wifi24 += clients_count;
		if (ifaces->frequency > 5000)
			wifi5 += clients_count;
next_statistics: ;
		void *freeptr = ifaces;
		ifaces = ifaces->next;
		free(freeptr);
	}

	//TODO maybe skip: if (wifi24 >  0 || wifi5 > 0) {
	json_object_object_add(clients, "wifi24", json_object_new_int(wifi24));
	json_object_object_add(clients, "wifi5", json_object_new_int(wifi5));
	json_object_object_add(result, "clients", clients);
	//}

	return result;
}

static struct json_object *respondd_provider_neighbours(void) {
	struct json_object *result, *wireless, *station;
	struct iface_list *ifaces;

	result = json_object_new_object();
	if (!result)
		return NULL;

	wireless = json_object_new_object();
	if (!wireless) {
		json_object_put(result);
		return NULL;
	}

	ifaces = get_ifaces();

	while (ifaces != NULL) {

		if(ifaces->type != NL80211_IFTYPE_ADHOC)
			goto next_neighbours;

		station = json_object_new_object();
		if (!station)
			goto next_neighbours;

		if (!get_neighbours(station, ifaces->ifx)) {
			json_object_put(station);
			goto next_neighbours;
		}

		if (ifaces->frequency)
			json_object_object_add(station, "frequency", json_object_new_int(ifaces->frequency));

		json_object_object_add(wireless, ifaces->mac_addr, station);

next_neighbours: ;
		void *freeptr = ifaces;
		ifaces = ifaces->next;
		free(freeptr);
	}


	json_object_object_add(result, "wifi", wireless);
	return result;
}

const struct respondd_provider_info respondd_providers[] = {
	{"statistics", respondd_provider_statistics},
	{"neighbours", respondd_provider_neighbours},
	{0, 0},
};
