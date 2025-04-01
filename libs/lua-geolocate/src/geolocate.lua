local iwinfo = require 'iwinfo'
local json = require 'jsonc'
local ubus = require 'ubus'


local function canon_bssid(bssid)
	return bssid:upper():gsub(':', '')
end

-- Iterates over all active WLAN interfaces
-- Returning true from the callback function will skip all remaining
-- interfaces of the same radio
local function foreach_radio(f)
	local uconn = assert(ubus.connect(), 'failed to connect to ubus')
	local status = uconn:call('network.wireless', 'status', {})
	ubus.close(uconn)

	for _, radio in pairs(status) do
		for _, iface in ipairs(radio.interfaces) do
			if f(iface.ifname) then
				break
			end
		end
	end
end

local function receive_json(request)
	local f = assert(io.popen(string.format("exec wget -T 15 -q -O- '%s'", request)), 'failed to run wget')
	local data = f:read('*a')
	f:close()

	return json.parse(data)
end

local function locate(blacklist)
	local done_bssids = {}
	for _, bssid in ipairs(blacklist or {}) do
		done_bssids[canon_bssid(bssid)] = true
	end

	local found_bssids = {}

	foreach_radio(function(ifname)
		local iw = iwinfo[iwinfo.type(ifname)]
		if not iw then
			-- Skip other ifaces of this radio, as they
			-- will have the same type
			return true
		end

		local scanlist = iw.scanlist(ifname)
		if not scanlist then
			return false
		end

		for _, entry in ipairs(scanlist) do
			local bssid = canon_bssid(entry.bssid)
			if not done_bssids[bssid] then
				table.insert(found_bssids, bssid)
				done_bssids[bssid] = true
			end
		end

		return true
	end)

	assert(#found_bssids >= 12, 'insufficient BSSIDs found')

	local data = receive_json('http://openwifi.su/api/v1/bssids/' .. table.concat(found_bssids, ','))
	assert(type(data) == 'table' and data.lon and data.lat, 'location not available')

	return data
end


local geolocate = {}

function geolocate.locate(blacklist)
	local ok, result = pcall(locate, blacklist)
	if ok then
		return result
	else
		return nil, result
	end
end

return geolocate
