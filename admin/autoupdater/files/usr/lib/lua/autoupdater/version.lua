module 'autoupdater.version'


-- version comparison is based on dpkg code
local function isdigit(s, i)
  local c = s:sub(i, i)
  return c and c:match('^%d$')
end

local function char_value(s, i)
  return s:byte(i, i) or 0
end

local function char_order(s, i)
  local c = s:sub(i, i)

  if c == '' or c:match('^%d$') then
    return 0
  elseif c:match('^%a$') then
    return c:byte()
  elseif c == '~' then
    return -1
  else
    return c:byte() + 256
  end
end

-- returns true when a is a higher version number than b
function newer_than(a, b)
  local apos = 1
  local bpos = 1

  while apos <= a:len() or bpos <= b:len() do
    local first_diff = 0

    while (apos <= a:len() and not isdigit(a, apos)) or (bpos <= b:len() and not isdigit(b, bpos)) do
      local ac = char_order(a, apos)
      local bc = char_order(b, bpos)

      if ac ~= bc then
	return ac > bc
      end

      apos = apos + 1
      bpos = bpos + 1
    end

    while a:sub(apos, apos) == '0' do
      apos = apos + 1
    end

    while b:sub(bpos, bpos) == '0' do
      bpos = bpos + 1
    end

    while isdigit(a, apos) and isdigit(b, bpos) do
      if first_diff == 0 then
	first_diff = char_value(a, apos) - char_value(b, bpos)
      end

      apos = apos + 1
      bpos = bpos + 1
    end

    if isdigit(a, apos) then
      return true
    end

    if isdigit(b, bpos) then
      return false
    end

    if first_diff ~= 0 then
      return first_diff > 0
    end
  end

  return false
end
