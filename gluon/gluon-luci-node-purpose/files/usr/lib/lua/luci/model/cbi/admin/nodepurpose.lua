local f, s, o, role
local uci = luci.model.uci.cursor()
local config = 'gluon-node-info'

-- where to read the configuration from
local role = uci:get(config, uci:get_first(config, "purpose"), "role")

f = SimpleForm("purpose", "Verwendungszweck")
f.reset = false
f.template = "admin/expertmode"
f.submit = "Fertig"

s = f:section(SimpleSection, nil, [[
Wenn dein Freifunk-Router eine besondere Rolle im Freifunk Netz einnimmt, kannst du diese hier angeben.
Das kann z.B. die "Backbone" Rolle sein, damit er auf der Freifunk-Knotenkarte entsprechend gekennzeichnet wird.
Setze die Rolle nur, wenn du weißt was du machst. Informiere dich bitte vorher, was die entsprechenden Rollen im
Freifunk-Netz bewirken.
]])

o = s:option(ListValue, "role", "Rolle")
o.default = role
o.rmempty = false
o:value("node", "Normaler Knoten")
o:value("backbone", "Backbone Knoten")

function f.handle(self, state, data)
  if state == FORM_VALID then
    uci:set(config, uci:get_first(config, "purpose"), data.role)

    uci:save(config)
    uci:commit(config)
  end
end

return f
