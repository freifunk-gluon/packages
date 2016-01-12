local nixio = require 'nixio'
local fs = require 'nixio.fs'
local util = require 'nixio.util'


module('autoupdater.util', package.seeall)


-- Executes a command in the background, without parsing the command through a shell (in contrast to os.execute)
function exec(...)
  local pid, errno, error = nixio.fork()
  if pid == 0 then
    nixio.execp(...)
    os.exit(127)
  elseif pid > 0 then
    local wpid, status, code = nixio.waitpid(pid)
    return wpid and status == 'exited' and code
  else
    return pid, errno, error
  end
end


-- Executes a command in the background, returning its PID and a pipe connected to the command's standard input
function popen(...)
  local inr, inw = nixio.pipe()
  local pid = nixio.fork()

  if pid > 0 then
    inr:close()

    return pid, inw
  elseif pid == 0 then
    nixio.dup(inr, nixio.stdin)

    inr:close()
    inw:close()

    nixio.execp(...)
    os.exit(127)
  end
end


-- Executes all executable files in a directory
function run_dir(dir)
  local function is_ok(entry)
    if entry:sub(1, 1) == '.' then
      return false
    end

    local file = dir .. '/' .. entry
    if fs.stat(file, 'type') ~= 'reg' then
      return false
    end
    if not fs.access(file, 'x') then
      return false
    end

    return true
  end

  local files = util.consume(fs.dir(dir))
  if not files then
    return
  end

  table.sort(files)

  for _, entry in ipairs(files) do
    if is_ok(entry) then
      exec(dir .. '/' .. entry)
    end
  end
end


-- Seeds Lua's random generator from /dev/urandom
function randomseed()
  local f = io.open('/dev/urandom', 'r')
  local b1, b2, b3, b4 = f:read(4):byte(1, 4)
  f:close()

  -- The and is necessary as Lua on OpenWrt doesn't like integers over 2^31-1
  math.randomseed(nixio.bit.band(b1*0x1000000 + b2*0x10000 + b3*0x100 + b4, 0x7fffffff))
end


-- Takes a date and time in RFC3339 format and returns a Unix timestamp
function parse_date(date)
  local year, month, day, hour, minute, second, tzs, tzh, tzm = date:match('^(%d%d%d%d)%-(%d%d)%-(%d%d) (%d%d):(%d%d):(%d%d)([%+%-])(%d%d):(%d%d)$')
  if not year then
    return nil
  end

  local a = math.floor((14 - month)/12)
  local y = year - a
  local m = month + 12*a - 3

  -- Based on a well-known formula for Julian dates
  local days = day + math.floor((153*m + 2)/5) + 365*y + math.floor(y/4) - math.floor(y/100) + math.floor(y/400) - 719469
  local time = hour*3600 + minute*60 + second
  local tz = tzh*3600 + tzm*60

  if tzs == '-' then
    tz = -tz
  end

  return days * 86400 + time - tz
end
