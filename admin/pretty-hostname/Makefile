include $(TOPDIR)/rules.mk

PKG_NAME:=pretty-hostname
PKG_VERSION:=1

PKG_BUILD_DIR := $(BUILD_DIR)/$(PKG_NAME)

include $(INCLUDE_DIR)/package.mk

define Package/pretty-hostname
  SECTION:=admin
  CATEGORY:=Administration
  DEPENDS:=+uci
  TITLE:=Manage a pretty hostname in addition to the actual one
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
endef

define Build/Configure
endef

define Build/Compile
endef

define Package/pretty-hostname/install
	$(INSTALL_DIR) $(1)/bin
	$(INSTALL_BIN) ./files/pretty-hostname $(1)/bin/

	$(INSTALL_DIR) $(1)/usr/lib/lua
	$(INSTALL_DATA) ./files/pretty_hostname.lua $(1)/usr/lib/lua/
endef

$(eval $(call BuildPackage,pretty-hostname))
