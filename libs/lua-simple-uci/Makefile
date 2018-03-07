include $(TOPDIR)/rules.mk

PKG_NAME:=lua-simple-uci
PKG_VERSION:=1

PKG_LICENSE:=APL2

include $(INCLUDE_DIR)/package.mk

define Package/lua-simple-uci
	SECTION:=libs
	CATEGORY:=Libraries
	TITLE:=Useful functions for accessing uci using lua
	DEPENDS:=+libuci-lua
endef

define Build/Compile
endef

define Package/lua-simple-uci/install
	$(INSTALL_DIR) $(1)/usr/lib/lua
	$(INSTALL_DATA) src/simple-uci.lua $(1)/usr/lib/lua/
endef

$(eval $(call BuildPackage,lua-simple-uci))
