module("luci.controller.ffhl.index", package.seeall)


function index()
  local uci_state = luci.model.uci.cursor_state()

  if uci_state:get_first("ffhl", "wizard", "running", "0") == "1" then
    local root = node()
    if not root.target then
      root.target = alias("wizard", "welcome")
      root.index = true
    end
  end
end

