local assert = assert
local string = string

module 'pretty_hostname'

local function get_system(uci)
	local system
	uci:foreach('system', 'system',
		function(s)
			system = s
			return false
		end
	)
	return assert(system, 'unable to find system section')
end

function get(uci)
	local system = get_system(uci)

	return system.pretty_hostname or system.hostname
end

function set(uci, pretty_hostname)
	local system = get_system(uci)['.name']

	local hostname = string.gsub(pretty_hostname, '[^a-zA-Z0-9%-]', '')
	hostname = string.gsub(hostname, '%-+', '-')
	hostname = string.gsub(hostname, '^%-', '')
	hostname = string.sub(hostname, 1, 63)
	hostname = string.gsub(hostname, '%-$', '')

	if hostname == '' then
		hostname = 'localhost'
	end

	if hostname == pretty_hostname then
		uci:delete('system', system, 'pretty_hostname')
	else
		uci:set('system', system, 'pretty_hostname', pretty_hostname)
	end

	uci:set('system', system, 'hostname', hostname)
end
