local msg = [[<p>
               <%= luci.template.render_string(site.config_mode.msg_pubkey) %>
             </p>
             <div class="the-key">
               # <%= hostname %>
               <br/>
               <%= pubkey %>
             </div>]]

local uci = luci.model.uci.cursor()
local meshvpn_enabled = uci:get("fastd", "mesh_vpn", "enabled", "0")

if meshvpn_enabled ~= "1" then
  return nil
else
  local util = require "luci.util"
  local site = require 'gluon.site_config'
  local sysconfig = require 'gluon.sysconfig'

  local pubkey = util.exec("/etc/init.d/fastd show_key " .. "mesh_vpn")
  local hostname = uci:get_first("system", "system", "hostname")

  return function ()
           luci.template.render_string(msg, { pubkey=pubkey
                                            , hostname=hostname
                                            , site=site
                                            , sysconfig=sysconfig
                                            })
         end
end
