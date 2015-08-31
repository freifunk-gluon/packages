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

-- returns a boolean whether to delete the current section (optional)
function Cursor.delete_all(self, config, stype, comparator)
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

function Cursor.section(self, config, type, name, values)
	local stat = true
	if name then
		stat = self:set(config, name, type)
	else
		name = self:add(config, type)
		stat = name and true
	end

	if stat and values then
		stat = self:tset(config, name, values)
	end

	return stat and name
end

function Cursor.tset(self, config, section, values)
	local stat = true
	for k, v in pairs(values) do
		if k:sub(1, 1) ~= "." then
			stat = stat and self:set(config, section, k, v)
		end
	end
	return stat
end

function Cursor.get_bool(self, ...)
	local val = self:get(...)
	return ( val == "1" or val == "true" or val == "yes" or val == "on" )
end

function Cursor.get_list(self, config, section, option)
	if config and section and option then
		local val = self:get(config, section, option)
		return ( type(val) == "table" and val or { val } )
	end
	return nil
end

function Cursor.get_first(self, conf, stype, opt, def)
	local rv = def

	self:foreach(conf, stype,
		function(s)
			local val = not opt and s['.name'] or s[opt]

			if type(def) == "number" then
				val = tonumber(val)
			elseif type(def) == "boolean" then
				val = (val == "1" or val == "true" or
				       val == "yes" or val == "on")
			end

			if val ~= nil then
				rv = val
				return false
			end
		end)

	return rv
end

function Cursor.add_to_set(self, config, section, option, value, remove)
	local list = self:get_list(config, section, option)

	if not list then
		return false
	end

	local set = {}
	for _, l in ipairs(list) do
		set[l] = true
	end

	if remove then
		set[value] = nil
	else
		set[value] = true
	end

	list = {}
	for k, _ in pairs(set) do
		table.insert(list, k)
	end

	if next(list) == nil then
		return self:delete(config, section, option)
	else
		return self:set(config, section, option, list)
	end
end

function Cursor.remove_from_set(self, config, section, option, value)
	self:add_to_set(config, section, option, value, true)
end
