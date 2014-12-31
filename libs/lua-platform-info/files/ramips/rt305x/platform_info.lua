local f = io.popen('. /lib/functions.sh; . /lib/ramips.sh; ramips_board_detect; echo "$RAMIPS_BOARD_NAME"; echo "$RAMIPS_MODEL"')
local board_name, model = f:read("*a"):match('([^\n]+)\n([^\n]+)')
f:close()


module 'platform_info'


-- The OpenWrt target
function get_target()
   return 'ramips'
end

-- The OpenWrt subtarget or nil
function get_subtarget()
   return 'rt305x'
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
   return (model:lower():gsub('[^%w]+', '-'):gsub('%-+$', ''))
end
