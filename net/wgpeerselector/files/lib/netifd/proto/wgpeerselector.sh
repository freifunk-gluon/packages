#!/bin/sh

. /lib/functions.sh
. ../netifd-proto.sh
init_proto "$@"

proto_wgpeerselector_init_config() {
	proto_config_add_string 'unix_group'
}

proto_wgpeerselector_setup() {
	local config="$1"
	local iface="$2"
	local unix_group

	json_get_vars unix_group

	(proto_add_host_dependency "$config" '' "$iface")

	proto_init_update "$iface" 1
	proto_send_update "$config"

	proto_run_command "$config" wgpeerselector \
		-i "$iface" ${unix_group:+--group "$unix_group"}
}


proto_wgpeerselector_teardown() {
	local config="$1"
	proto_kill_command "$config"
}

add_protocol wgpeerselector
