wgpeerselector
==============

The wireguard peer selector is a daemon allowing to connect to only one
wireguard peer of a set of N peers. It does that by randomingly picking one
peer, trying to connect and on failure trying to connect to the next peer.
It is comparable to setting the "peer limit" to one in fastd. The daemon
is written in lua and hardwired to OpenWrt mechanisms.

Features:
- Status socket via UBUS.
- Routes for "Allowed IPs" are installed to the kernel.
- Synchronize time via NTP.
    - Currently necessary in wireguard to reestablish a connection after
      rebooting. [Reference](https://lists.zx2c4.com/pipermail/wireguard/2019-February/003850.html)
- Can change its unix group.

How does it work?
-----------------

The daemon is configured using its netifd proto `wgpeerselector`. The wireguard
interface is set up by the standard wireguard netifd proto `wireguard`.
Configuring peers via the `wireguard` proto is not necessary because this will
be done by wgpeerselector based on its uci config:

/etc/config/network:
```
config interface 'vpn'
	option proto 'wireguard'
	option fwmark '1'
	option private_key 'YOUR_PRIVATE_KEY_GOES_HERE'
	list addresses 'fe80::02a4:18ff:fe7e:a10d'
	option disabled '0'

config interface 'vpn_peerselector'
	option proto 'wgpeerselector'
	option transitive '1'
	option ifname 'vpn'
	option unix_group 'YOUR_UNIX_GROUP_GOES_HERE'   # this is optional
```

/etc/config/wgpeerselector:
```
config peer 'test_unknown_a'
	option enabled '1'
	option public_key 'mIDOdscl2R3Dq+YthxdTvvtH4D53VhawzEnmet+E7W0='
	list allowed_ips 'fe80::1/128'
	option ifname 'vpn'
	option endpoint 'hostname.example.com:51820'
```

Status information can be obtained via ubus:
```
root@platzhalter-525400123456:~# ubus call wgpeerselector.vpn status
{
	"peers": {
		"test_unknown_a": false,
		"test_unknown_b": false,
		"test_sn07": {
			"established": 1490
		}
	}
}
```

Algorithm:
----------

A connection is considered as "established" if the latest handshakes was
within the last 2.5 minutes.

Installing a peer:
1. Resolve its endpoint if necessary.
2. Randomly pick a remote IP.
3. Install the peer with that ip to the kernel.
4. Wait 5 seconds.
5. Check whether the connection is established.
    - If not, remove the peer from the kernel again.

Main Loop:
- Wait for the inteface, if not existing.
- Try to synchronize time via NTP, if not done yet.
- If no connection is established:
	- Randomly pick a peer.
	- Try to establish a connection.
- If a connection is established:
	- Check every 5 seconds if the connection is still established.

Picking peers is done in such a way that all peers are tried once
before one peer is attempted for the second time. The same goes for the
DNS resolultion of the endpoints.

Warning: This algorithm only works if something is causing traffic into
the wireguard interface. This is due to the fact, that wireguard is only
doing handshakes, if it has to. While running mesh protocols on top of the
wireguard interface this is usually not a problem, since those protocols
periodically send Hello, OGM, ... packets.
