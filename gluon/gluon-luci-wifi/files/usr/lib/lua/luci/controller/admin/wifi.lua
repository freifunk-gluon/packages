module("luci.controller.admin.wifi", package.seeall)

function index()
	entry({"admin", "wifi"}, cbi("admin/wifi"), "WLAN", 10)
end
