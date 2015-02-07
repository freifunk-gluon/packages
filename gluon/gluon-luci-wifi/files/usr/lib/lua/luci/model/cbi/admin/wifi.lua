local f, s, o
local config = "wireless"
local uci = luci.model.uci.cursor()
local radios24 = {}
local radios5 = {}
local primary_iface = 'wan_radio0'
local private_ssid = uci:get(config, primary_iface, "ssid")
local countries = {{"Belgien", "BE"}, {"Bolivien", "BO"}, {"Dänemark", "DK"}, {"Deutschland", "DE"}, {"Frankreich", "FR"}, {"Italien", "IT"}, {"Niederlande", "NL"}, {"Österreich", "AT"}, {"Polen", "PL"}, {"Schweden", "SE"}, {"Schweiz", "CH"}, {"Tschechische Republik", "CZ"}, {"USA", "US"}}

-- look for wifi interfaces and add them to the specific array
uci:foreach(config, 'wifi-device',
    function(s)
        --get the hwmode to separate 2.4 GHz and 5 GHz radios
        local hwmode = uci:get(config, s['.name'], 'hwmode')
        if hwmode == '11g' or hwmode == '11ng' then --if 2.4 GHz
            table.insert(radios24, s['.name'])
        elseif hwmode == '11a' or hwmode == '11na' then --if 5 GHz
            table.insert(radios5, s['.name'])
        end
    end
)
local radios_type = {{"2,4 GHz","24",radios24},{"5 GHz", "5", radios5}}
f = SimpleForm("wifi", "WLAN-Einstellungen")
f.reset = false
f.template = "admin/expertmode"
f.submit = "Speichern"

s = f:section(SimpleSection, nil, [[
In diesem Abschnitt hast du die Möglichkeit die SSID für das Clientnetzwerk zu ändern (nur für Profis!), die maximale Sendeleistung und Reichweite festzusetzen,
die SSIDs des Client- und des Mesh-Netzes zu de/aktivieren, die Region auszuwählen und die Preisgabe der Region in den Beacons abzuschalten (wird nicht empfohlen).
]])
-- show options for 2.4/5 GHz devices
for i=1,#radios_type do
    local rtype = radios_type[i]
    if table.getn(rtype[3]) > 0 then
        s = f:section(SimpleSection, rtype[1], nil)
        for index, radio in ipairs(rtype[3]) do
        -- ssid for client network
        local ssid = uci:get(config, "client_" .. radio, "ssid")
        if ssid then
            o = s:option(Value, 'ssid' .. rtype[2] .. index, "AP-SSID")
            o.default = ssid
            o.rmempty = false
        end

        -- tx power 
        o = s:option(Value, 'txpower' .. rtype[2] .. index, "Sendeleistung (in dBm)")
        local txpower = uci:get(config, radio, "txpower")
        if not txpower then txpower="30" end
        o.default = txpower
        o.datatype = "uinteger"
        o.rmempty = false
        
        -- distance
        o = s:option(Value, 'distance' .. rtype[2] .. index, "Reichweite (in Metern)")
        local distance = uci:get(config, radio, "distance")
        if not distance then distance="100" end
        o.default = distance
        o.datatype = "uinteger"
        o.rmempty = false

        -- country code
        o=s:option(ListValue, "country" .. rtype[2] .. index, "Region")
        o.default=uci:get(config, radio, "country")
        for i=1,#countries do o:value(countries[i][2],countries[i][1]) end

        -- IE in beacons
        o = s:option(Flag, 'country_ie' .. rtype[2] .. index, "IE senden (empfohlen)")
        local country_ie = uci:get(config, radio, "country_ie")
        if not country_ie then country_ie="1" end
        o.default = country_ie
        o.rmempty = false

        -- client network switch
        o = s:option(Flag, 'clientbox' .. rtype[2] .. index, "Client-Netz aktivieren")
        o.default = (uci:get_bool(config, 'client_' .. radio, "disabled")) and o.disabled or o.enabled
        o.rmempty = false

        -- mesh network switch
        o = s:option(Flag, 'meshbox' .. rtype[2] .. index, "Mesh-Netz aktivieren")
        o.default = (uci:get_bool(config, 'mesh_' .. radio, "disabled")) and o.disabled or o.enabled
        o.rmempty = false
        end
    end
end

-- show options for private wifi
s = f:section(SimpleSection, "Privates WLAN", [[
Dein Freifunk-Router kann ebenfalls die Reichweite deines privaten Netzes erweitern.
Hierfür wird der WAN-Port mit einem separatem WLAN gebridged.
Diese Funktionalität ist völlig unabhängig von Freifunk.
Beachte, dass du nicht gleichzeitig das Meshen über den WAN Port aktiviert haben solltest.
]])

-- private wifi switch
o = s:option(Flag, "private_enabled", "Aktiviert")
o.default = (private_ssid and not uci:get_bool(config, primary_iface, "disabled")) and o.enabled or o.disabled
o.rmempty = false

-- private wifi ssid
o = s:option(Value, "private_ssid", "Name (SSID)")
o.default = private_ssid

-- private wifi key
o = s:option(Value, "private_key", "Schlüssel", "8-63 Zeichen")
o.datatype = "wpakey"
o.default = uci:get(config, primary_iface, "key")

-- handle submitted changes
function f.handle(self, state, data)
    if state == FORM_VALID then
        for i=1,#radios_type do
            local rtype = radios_type[i]
            for index, radio in ipairs(rtype[3]) do
                -- set client network ssid
                uci:set(config, "client_" .. radio, "ssid", data["ssid" .. rtype[2] .. index])

                -- set tx power
                uci:set(config, radio, "txpower", data["txpower" .. rtype[2] .. index])
                    
                -- set country code
                uci:set(config, radio, "country", data["country" .. rtype[2] .. index])

                -- set distance
                uci:set(config, radio, "distance", data["distance" .. rtype[2] .. index])

                -- set IE in beacons
                uci:set(config, radio, "country_ie", data["country_ie" .. rtype[2] .. index])

                -- set client network switch
                if data["clientbox" .. rtype[2] .. index] == '0' then
                    uci:set(config, 'client_' .. radio, "disabled", 'true')
                else
                    uci:set(config, 'client_' .. radio, "disabled", 'false')
                end

                -- set mesh network switch
                if data["meshbox" .. rtype[2] .. index] == '0' then
                    uci:set(config, 'mesh_' .. radio, "disabled", 'true')
                else
                    uci:set(config, 'mesh_' .. radio, "disabled", 'false')
                end

            end
        end
   
        uci:foreach(config, "wifi-device",
            function(s)
                local device = s['.name']
                local name   = "wan_" .. device
                if data.private_enabled == '1' then
                    -- set up WAN wifi-iface
                    local t      = uci:get_all(config, name) or {}
                    t.device     = device
                    t.network    = "wan"
                    t.mode       = 'ap'
                    t.encryption = 'psk2'
                    t.ssid       = data.private_ssid
                    t.key        = data.private_key
                    t.disabled   = "false"
                    uci:section(config, "wifi-iface", name, t)
                else
                    -- disable WAN wifi-iface
                    uci:set(config, name, "disabled", "true")
                end

        end)
        -- save and commit changes
        uci:save(config)
        uci:commit(config)

    end
end

return f
