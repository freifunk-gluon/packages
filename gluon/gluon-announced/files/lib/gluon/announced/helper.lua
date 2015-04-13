local announce = require 'gluon.announce'

local json = require 'luci.json'
local ltn12 = require 'luci.ltn12'

local announce_base = '/lib/gluon/announce/'

local function collect(type)
  return announce.collect_dir(announce_base .. type .. '.d')
end

function request(query)
  if query:match('^nodeinfo') then
    return json.encode(collect('nodeinfo'))
  end

  local m = query:match('^GET ([a-z ]+)')
  if m then
    local data = {}

    for q in m:gmatch('([a-z]+)') do
      local ok, val = pcall(collect, q)
      if ok then
        data[q] = val
      end
    end

    if next(data) then
      return deflate(json.encode(data))
    end
  end

  return nil
end
