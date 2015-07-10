local function read_line(file)
  local f = io.open(file)
  local ret = f:read('*line')
  f:close()
  return ret
end

local board_name = read_line('/tmp/sysinfo/board_name')
local model = read_line('/tmp/sysinfo/model')

module 'platform_info'


-- The OpenWrt target
function get_target()
   return 'brcm2708'
end

-- The OpenWrt subtarget or nil
function get_subtarget()
   return 'bcm2709'
end

-- The board name
function get_board_name()
   return board_name
end

-- The model name
function get_model()
   return model
end

-- The image name for sysupgrades
function get_image_name()
   return nil
end
