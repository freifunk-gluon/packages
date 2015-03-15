local model

for line in io.lines('/proc/cpuinfo') do
  model = line:match('^model name%s*:%s*(.+)$')
  if model then
    break
  end
end


local image_name = 'x86-generic'

local f = io.open('/sys/class/dmi/id/sys_vendor')
if f then
  local vendor = f:read('*line')
  f:close()

  if vendor then
    if vendor:match('^VMware') or vendor:match('^VMW') then
      image_name = 'x86-vmware'
    elseif vendor:match('^innotek GmbH') then
      image_name = 'x86-virtualbox'
    end
  end
end


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
  return model
end

-- The image name for sysupgrades
function get_image_name()
  return image_name
end
