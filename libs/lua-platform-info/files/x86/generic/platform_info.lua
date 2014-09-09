module 'platform_info'


-- The OpenWrt target
function get_target()
   return 'x86'
end

-- The OpenWrt subtarget or nil
function get_subtarget()
   return 'generic'
end

-- The board name
function get_board_name()
   return nil
end

-- The model name
function get_model()
   return nil
end

-- The image name for sysupgrades
function get_image_name()
   return nil
end
