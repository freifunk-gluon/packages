local f, s, o, p, q
local uci = luci.model.uci.cursor()
local config = 'wireless'
 
--set the heading, button and stuff 
f = SimpleForm("wifi", "WLAN-Config")
f.reset = false
f.template = "admin/expertmode"
f.submit = "Speichern"

-- text, which describes what the package does to the user
s = f:section(SimpleSection, nil, [[
In diesem Abschnitt hast du die Möglichkeit die SSIDs des Client- und des
Mesh-Netzes zu deaktivieren. Bitte lass die SSID des Mesh-Netzes aktiviert, 
damit sich auch andere Router über dich mit dem Freifunk verbinden können.
]])
 

local radios = {}

-- look for wifi interfaces and add them to the array
uci:foreach('wireless', 'wifi-device',
function(s)
	table.insert(radios, s['.name'])
end
)

--add a client and mesh checkbox  for each interface
for index, radio in ipairs(radios) do
 	--get the hwmode to seperate 2.4GHz and 5Ghz radios
	local hwmode = uci:get('wireless', radio, 'hwmode')
	
	if hwmode == '11g' or hwmode == '11ng' then --if 2.4GHz
	 	
	 	p = f:section(SimpleSection, [[2,4GHz Wlan]], nil)

	 	--box for the clientnet
		o = p:option(Flag, 'clientbox' .. index, "Client-Netz aktivieren")
		o.default = (uci:get_bool(config, 'client_' .. radio, "disabled")) and o.disabled or o.enabled
		o.rmempty = false
		--box for the meshnet 
		o = p:option(Flag, 'meshbox' .. index, "Mesh-Netz aktivieren")
		o.default = (uci:get_bool(config, 'mesh_' .. radio, "disabled")) and o.disabled or o.enabled
		o.rmempty = false
	 
	elseif hwmode == '11a' or hwmode == '11na' then --if 5GHz
		
		q = f:section(SimpleSection, [[5GHz Wlan]], nil)

		--box for the clientnet
		o = q:option(Flag, 'clientbox' .. index, "Client-Netz aktivieren")
		o.default = (uci:get_bool(config, 'client_' .. radio, "disabled")) and o.disabled or o.enabled
		o.rmempty = false
		--box for the meshnet
		o = q:option(Flag, 'meshbox' .. index, "Mesh-Netz aktivieren")
		o.default = (uci:get_bool(config, 'mesh_' .. radio, "disabled")) and o.disabled or o.enabled
		o.rmempty = false
	 
	end
end
 
--if the save-button is pushed
function f.handle(self, state, data)
	if state == FORM_VALID then
	 
		for index, radio in ipairs(radios) do

			local clientstate, meshstate
			-- get the data from the boxes and invert it
			if data["clientbox"..index] == '0' then
				clientenabled = 'true'
			else
				clientenabled = 'false'
			end
			-- write the data to the config file
			uci:set(config, 'client_' .. radio, "disabled", clientenabled)

			
			if data["meshbox"..index] == '0' then                                                                      
			    meshenabled = 'true'                                                                    
			else                                                                                          
			    meshenabled = 'false'                                                                   
			end 
						
			uci:set(config, 'mesh_' .. radio, "disabled", meshenabled)

		end

	uci:save(config)
	uci:commit(config)
	end
end
 
return f
