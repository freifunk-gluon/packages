local io = io
local math = math
local nixio = require 'nixio'


module 'autoupdater.util'


-- Executes a command in the background, returning its PID and a pipe connected to the command's standard input
function popen(command)
  local inr, inw = nixio.pipe()
  local pid = nixio.fork()

  if pid > 0 then
    inr:close()

    return pid, inw
  elseif pid == 0 then
    nixio.dup(inr, nixio.stdin)

    inr:close()
    inw:close()

    nixio.exec('/bin/sh', '-c', command)
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
