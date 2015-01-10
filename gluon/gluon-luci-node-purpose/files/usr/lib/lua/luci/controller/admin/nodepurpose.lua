module("luci.controller.admin.nodepurpose", package.seeall)

function index()
	entry({"admin", "nodepurpose"}, cbi("admin/nodepurpose"), "Verwendungszweck", 20)
end
