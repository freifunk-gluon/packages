-- Copyright 2008 Steven Barth <steven@midlink.org>
-- Copyright 2015 Nils Schneider <nils@nilsschneider.net>
-- Licensed to the public under the Apache License 2.0.
--
-- This is basically everything useful from luci.model.uci
-- without any luci dependency.

local uci   = require "uci"
local table = require "table"

local getmetatable = getmetatable
local next, pairs, ipairs = next, pairs, ipairs
local type, tonumber = type, tonumber

module "simple-uci"

cursor = uci.cursor

APIVERSION = uci.APIVERSION

local Cursor = getmetatable(cursor())


local uciset = Cursor.set

function Cursor:set(config, section, option, value)
	if value ~= nil and not (type(value) == 'table' and #value == 0) then
		if type(value) == 'boolean' then
			value = value and '1' or '0'
		end

		return uciset(self, config, section, option, value)
	else
		self:delete(config, section, option)
		return true
	end
end

-- returns a boolean whether to delete the current section (optional)
function Cursor:delete_all(config, stype, comparator)
	local del = {}

	if type(comparator) == "table" then
		local tbl = comparator
		comparator = function(section)
			for k, v in pairs(tbl) do
				if section[k] ~= v then
					return false
				end
			end
			return true
		end
	end

	local function helper (section)

		if not comparator or comparator(section) then
			del[#del+1] = section[".name"]
		end
	end

	self:foreach(config, stype, helper)

	for i, j in ipairs(del) do
		self:delete(config, j)
	end
end

function Cursor:section(config, type, name, values)
	local stat = true
	if name then
		stat = uciset(self, config, name, type)
	else
		name = self:add(config, type)
		stat = name and true
	end

	if stat and values then
		stat = self:tset(config, name, values)
	end

	return stat and name
end

function Cursor:tset(config, section, values)
	local stat = true
	for k, v in pairs(values) do
		if k:sub(1, 1) ~= "." then
			stat = stat and self:set(config, section, k, v)
		end
	end
	return stat
end

function Cursor:get_bool(...)
	local val = self:get(...)
	return val == "1"
end

function Cursor:get_list(config, section, option)
	if config and section and option then
		local val = self:get(config, section, option)
		return ( type(val) == "table" and val or { val } )
	end
	return {}
end

function Cursor:get_first(conf, stype, opt, def)
	local rv = def

	self:foreach(conf, stype,
		function(s)
			local val = not opt and s['.name'] or s[opt]

			if type(def) == "number" then
				val = tonumber(val)
			elseif type(def) == "boolean" then
				val = val == "1"
			end

			if val ~= nil then
				rv = val
				return false
			end
		end)

	return rv
end

function Cursor:set_list(config, section, option, value)
	if config and section and option then
		return self:set(
			config, section, option,
			(type(value) == "table" and value or { value })
		)
	end
	return false
end
