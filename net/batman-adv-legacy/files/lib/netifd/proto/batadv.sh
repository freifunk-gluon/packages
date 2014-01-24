#!/bin/sh

. /lib/functions.sh
. ../netifd-proto.sh
init_proto "$@"

proto_batadv_init_config() {
	proto_config_add_string "mesh"
	proto_config_add_string "mesh_no_rebroadcast"
}

proto_batadv_setup() {
	local config="$1"
	local iface="$2"

	local mesh mesh_no_rebroadcast
	json_get_vars mesh mesh_no_rebroadcast

	echo "$mesh" > "/sys/class/net/$iface/batman_adv/mesh_iface"
	[ -n "$mesh_no_rebroadcast" ] && echo "$mesh_no_rebroadcast" > "/sys/class/net/$iface/batman_adv/no_rebroadcast"

	proto_init_update "$iface" 1
	proto_send_update "$config"
}

proto_batadv_teardown() {
	local config="$1"
	local iface="$2"

	echo "none" > "/sys/class/net/$iface/batman_adv/mesh_iface" || true
}

add_protocol batadv
