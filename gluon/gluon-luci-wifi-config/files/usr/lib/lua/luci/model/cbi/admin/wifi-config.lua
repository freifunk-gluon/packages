local f, s, o
local uci = luci.model.uci.cursor()
local config = 'wireless'
 
-- where to read the configuration from
 
f = SimpleForm("wifi", "WLAN-Config")
f.reset = false
f.template = "admin/expertmode"
f.submit = "Speichern"
 
s = f:section(SimpleSection, nil, [[
Viele Freifunk-Communitys betreiben ein sogenanntes Dachnetz. Das bedeutet, dass
manche Router sich über große Strecken miteinander verbinden, um viele kleine
Mesh-Netze miteinander zu verbinden. Um diese weiten Verbindungen effektiver zu
gestalten hast du hier die Möglichkeit die SSID, mit der sich die Clients verbinden,
zu deaktivieren.
]])
 
 
local radios = {}
 
uci:foreach('wireless', 'wifi-device',
function(s)
table.insert(radios, s['.name'])
end
)
 
for index, radio in ipairs(radios) do
 
local hwmode = uci:get('wireless', radio, 'hwmode')
if hwmode == '11g' or hwmode == '11ng' then
 
o = s:option(Flag, 'clientbox' .. index, "2,4GHz Client Netz aktivieren")
o.default = (uci:get_bool(config, 'client_' .. radio, "disabled")) and o.disabled or o.enabled
o.rmempty = false
 
o = s:option(Flag, 'meshbox' .. index, "2,4GHz Mesh Netz aktivieren")
o.default = (uci:get_bool(config, 'client_' .. radio, "disabled")) and o.disabled or o.enabled
o.rmempty = false
 
elseif hwmode == '11a' or hwmode == '11na' then
 
o = s:option(Flag, 'clientbox' .. index, "5GHz Client Netz aktivieren")
o.default = (uci:get_bool(config, 'client_' .. radio, "disabled")) and o.disabled or o.enabled
o.rmempty = false
 
o = s:option(Flag, 'meshbox' .. index, "5GHz Mesh Netz aktivieren")
o.default = (uci:get_bool(config, 'client_' .. radio, "disabled")) and o.disabled or o.enabled
o.rmempty = false
 
end
end
 
 
function f.handle(self, state, data)
if state == FORM_VALID then
 
for index, radio in ipairs(radios) do
 
local currentclient = 'client_' .. radio
local currentmesh = 'mesh_' .. radio
uci:set(config, currentclient, "disabled", not data["clientbox"..index])
uci:set(config, currentmesh, "disabled", not data["meshbox"..index])
 
end
uci:save(config)
uci:commit(config)
end
end
 
return f