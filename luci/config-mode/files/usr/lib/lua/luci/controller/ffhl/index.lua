module("luci.controller.ffhl.index", package.seeall)

function index()
  local uci = luci.model.uci.cursor()

  if uci:get_first("ffhl", "wizard", "enabled") == "1" then
    local root = node()
    if not root.target then
      root.target = alias("wizard", "welcome")
      root.index = true
    end
  end
end

