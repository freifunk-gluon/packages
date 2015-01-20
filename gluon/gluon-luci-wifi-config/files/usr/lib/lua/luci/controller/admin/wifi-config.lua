module("luci.controller.admin.wifi-config", package.seeall)

function index()
	entry({"admin", "wifi-config"}, cbi("admin/wifi-config"), "WLAN-Config", 20)
end
