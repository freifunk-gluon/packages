#!/bin/sh

if [ -f /lib/functions/jshn.sh ]; then
	. /lib/functions/jshn.sh
elif [ -f /usr/share/libubox/jshn.sh ]; then
	. /usr/share/libubox/jshn.sh
else
	echo "Error: jshn.sh not found!"
	exit 1
fi

. /lib/gluon/functions/model.sh
. /lib/gluon/functions/sysconfig.sh

# set defaults
[ -z "$ALFRED_DATA_TYPE" ] && ALFRED_DATA_TYPE=158
[ -z "$NET_IF" ] && NET_IF=br-client
[ -z "$MAX_WAIT" ] && MAX_WAIT=299

set -e

# To avoid mass flooding the network every five minutes with all clients
# simultaneously, wait for a random time between 0 and 300 seconds, but fixed
# for each device to maintain 5 minutes between updates.
# Calculated using first 3 hex digits of the primary MAC address' MD5 hash
DELAY=$((0x$(sysconfig primary_mac | md5sum | head -c3) * $MAX_WAIT / (16**3)))
sleep $DELAY

json_init
json_add_string "hostname" "$(uci get 'system.@system[0].hostname')"

if [ "$(uci -q get 'system.@system[0].share_location')" = 1 ]; then
json_add_object "location"
	json_add_double "latitude" "$(uci get 'system.@system[0].latitude')"
	json_add_double "longitude" "$(uci get 'system.@system[0].longitude')"
json_close_object # location
fi

json_add_object "software"
	json_add_object "firmware"
		json_add_string "base" "gluon"
		json_add_string "release" "$(cat /lib/gluon/release)"
	json_close_object # firmware

	if [ -x /usr/sbin/autoupdater ]; then
	json_add_object "autoupdater"
		json_add_string "branch" "$(uci -q get autoupdater.settings.branch)"
		json_add_boolean "enabled" "$(uci -q get autoupdater.settings.enabled)"
	json_close_object # autoupdater
	fi

	if [ -x /usr/bin/fastd ]; then
	json_add_object "fastd"
		json_add_string "version" "$(fastd -v | cut -d' ' -f2)"
		json_add_boolean "enabled" "$(uci -q get fastd.mesh_vpn.enabled)"
	json_close_object # fastd
	fi
json_close_object # software

json_add_object "hardware"
	json_add_string "model" "$(get_model)"
json_close_object # hardware

json_add_object "network"
	json_add_string "mac"   "$(sysconfig primary_mac)"
	json_add_array "addresses"
		for addr in $(ip -o -6 addr show dev "$NET_IF" | grep -oE 'inet6 [0-9a-fA-F:]+' | cut -d' ' -f2); do
			json_add_string "" "$addr"
		done
	json_close_array # adresses
json_close_object # network

json_add_object "statistics"
	json_add_int "uptime" "$(cut -d' ' -f1 /proc/uptime)"
	json_add_object "traffic"
		json_add_double "rx" "$(cat /sys/class/net/bat0/statistics/rx_bytes)"
		json_add_double "tx" "$(cat /sys/class/net/bat0/statistics/tx_bytes)"
	json_close_object # traffic
json_close_object # statistics

json_dump | tr -d '\n' | alfred -s "$ALFRED_DATA_TYPE"
