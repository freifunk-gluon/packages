local uci = luci.model.uci.cursor()

local nav = require "luci.tools.ffhl-wizard.nav"

local f = SimpleForm("meshvpn", "Mesh-VPN", "<p>Um deinen Freifunkknoten auch über das Internet mit dem Freifunk-Netzwerk zu verbinden, kann das Mesh-VPN aktiviert werden.\
Dies erlaubt es, den Freifunk-Knoten zu betreiben, auch wenn es keine anderen Knoten in deiner Umgebung gibt, mit denen eine WLAN-Verbindung möglich ist.</p>\
<p style="color:red">Hier fehlt ein Satz zur Sicherheit</p>\
<p>Damit das Mesh-VPN deine Internet-Verbindung nicht unverhältnismäßig auslastet, kann die Bandbreite begrenzt werden. Wenn du zum Beispiel eine DSL-16000-Leitung hast\
und maximal ein Viertel der Leitung zur Verfügung stellen willst, muss als Downstream-Bandbreite 4000 kbit/s eingetragen werden.</p>\
<p>Um das Freifunk-Netz nicht zu sehr auszubremsen, bitten wir darum, mindestens 1000 kbit/s im Downstream und 100 kbit/s im Upstream bereitzustellen.</p>")
f.template = "ffhl-wizard/wizardform"

meshvpn = f:field(Flag, "meshvpn", "Mesh-VPN aktivieren?")
meshvpn.default = string.format("%d", uci:get("fastd", "ffhl_mesh_vpn", "enabled", "0"))
meshvpn.rmempty = false

tc = f:field(Flag, "tc", "Bandbreitenbegrenzung aktivieren?")
tc.default = string.format("%d", uci:get_first("ffhl", "bandwidth", "enabled", "0"))
tc.rmempty = false

downstream = f:field(Value, "downstream", "Downstream-Bandbreite (kbit/s)")
downstream.value = uci:get_first("ffhl", "bandwidth", "downstream", "0")
upstream = f:field(Value, "upstream", "Upstream-Bandbreite (kbit/s)")
upstream.value = uci:get_first("ffhl", "bandwidth", "upstream", "0")

function f.handle(self, state, data)
  if state == FORM_VALID then
    local stat = false
    uci:set("fastd", "ffhl_mesh_vpn", "enabled", data.meshvpn)
    uci:save("fastd")
    uci:commit("fastd")

    uci:foreach("ffhl", "bandwidth", function(s)
            uci:set("ffhl", s[".name"], "upstream", data.upstream)
            uci:set("ffhl", s[".name"], "downstream", data.downstream)
            uci:set("ffhl", s[".name"], "enabled", data.tc)
            end
    )

    uci:save("ffhl")
    uci:commit("ffhl")

    if data.meshvpn == "1" then
      local secret = uci:get("fastd", "ffhl_mesh_vpn", "secret")
      if not secret or not secret:match("%x%x%x%x%x%x%x%x%x%x%x%x%x%x%x%x%x%x%x%x%x%x%x%x%x%x%x%x%x%x%x%x%x%x%x%x%x%x%x%x%x%x%x%x%x%x%x%x%x%x%x%x%x%x%x%x%x%x%x%x%x%x%x%x") then
        luci.sys.call("/etc/init.d/haveged start")
        local f = io.popen("fastd --generate-key --machine-readable", "r")
        local secret = f:read("*a")
        f:close()
        luci.sys.call("/etc/init.d/haveged stop")

        uci:set("fastd", "ffhl_mesh_vpn", "secret", secret)
        uci:save("fastd")
        uci:commit("fastd")

      end
      luci.http.redirect(luci.dispatcher.build_url("wizard", "meshvpn", "pubkey"))
    else
      nav.maybe_redirect_to_successor()
    end
  end

  return true
end

return f
