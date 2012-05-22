module("luci.controller.ffhl.index", package.seeall)

function index()
	entry({}, call("maybe_wizard"), "luebeck.freifunk.net", 20).dependent=false
end

function maybe_wizard()
	-- This function should figure out whether the wizard already ran succesfully
	-- and redirect to the wizard or the status page.
	
	luci.http.redirect(luci.dispatcher.build_url("wizard", "welcome"))
end
