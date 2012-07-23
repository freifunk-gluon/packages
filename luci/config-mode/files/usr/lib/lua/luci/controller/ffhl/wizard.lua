module("luci.controller.ffhl.wizard", package.seeall)

function index()
  local uci = luci.model.uci.cursor()
  if uci:get_first("ffhl", "wizard", "enabled") == "1" then
    entry({"wizard", "welcome"}, template("ffhl-wizard/welcome"), "Willkommen", 10).dependent=false
    entry({"wizard", "password"}, form("ffhl-wizard/password"), "Passwort", 20).dependent=false
    entry({"wizard", "hostname"}, form("ffhl-wizard/hostname"), "Hostname", 30).dependent=false
    entry({"wizard", "meshvpn"}, form("ffhl-wizard/meshvpn"), "Mesh-VPN", 40).dependent=false
    entry({"wizard", "meshvpn", "pubkey"}, template("ffhl-wizard/meshvpn-key"), "Mesh-VPN Key", 1).dependent=false
    entry({"wizard", "completed"}, template("ffhl-wizard/completed"), "Fertig", 50).dependent=false
    entry({"wizard", "completed", "reboot"}, call("reboot"), "reboot", 1).dependent=false
  end
end

function reboot()
  local uci = luci.model.uci.cursor()

  uci:foreach("ffhl", "wizard", function(s)
    uci:set("ffhl", s[".name"], "enabled", "0")
    end
  )

  uci:save("ffhl")
  uci:commit("ffhl")

  luci.sys.reboot()
end

