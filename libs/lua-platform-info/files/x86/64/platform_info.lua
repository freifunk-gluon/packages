local model

for line in io.lines('/proc/cpuinfo') do
  model = line:match('^model name%s*:%s*(.+)$')
  if model then
    break
  end
end


module 'platform_info'

-- The OpenWrt target
function get_target()
  return 'x86'
end

-- The OpenWrt subtarget or nil
function get_subtarget()
  return '64'
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
  return 'x86-64'
end
