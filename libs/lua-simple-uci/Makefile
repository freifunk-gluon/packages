#
# Copyright (C) 2015 Nils Schneider <nils@nilsschneider.net>

include $(TOPDIR)/rules.mk

PKG_NAME:=lua-simple-uci
PKG_VERSION:=1
PKG_RELEASE:=1

PKG_LICENSE:=APL2

include $(INCLUDE_DIR)/package.mk

define Package/lua-simple-uci
	SECTION:=libs
	CATEGORY:=Libraries
	TITLE:=Useful functions for accessing uci using lua
	MAINTAINER:=Nils Schneider <nils@nilsschneider.net>
	DEPENDS:=+libuci-lua
endef

define Build/Configure
endef

define Build/Compile
endef

define Package/lua-simple-uci/install
	$(INSTALL_DIR) $(1)/usr/lib/lua
	$(INSTALL_BIN) src/simple-uci.lua $(1)/usr/lib/lua/
endef

$(eval $(call BuildPackage,lua-simple-uci))
