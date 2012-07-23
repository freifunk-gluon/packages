local nav = require "luci.tools.ffhl-wizard.nav"

f = SimpleForm("password", "Administrator Passwort setzen", "Damit nur du Zugriff auf deinen Freifunkknoten hast, solltest du jetzt ein Passwort vergeben.</p><p>Bitte wähle ein sicheres Passwort. Sonst könnten Leute wilden Scheiß machen, den du nicht willst!")
f.template = "ffhl-wizard/wizardform"

pw1 = f:field(Value, "pw1", "Passwort")
pw1.password = true
pw1.rmempty = false

pw2 = f:field(Value, "pw2", "Wiederholung")
pw2.password = true
pw2.rmempty = false

function pw2.validate(self, value, section)
  return pw1:formvalue(section) == value and value
end

function f.handle(self, state, data)
  if state == FORM_VALID then
    local stat = luci.sys.user.setpasswd("root", data.pw1) == 0

    if stat then
            nav.maybe_redirect_to_successor()
            f.message = "Passwort geändert!"
    else
      f.errmessage = "Fehler!"
    end

    data.pw1 = nil
    data.pw2 = nil
  end

  return true
end

return f
