include $(TOPDIR)/rules.mk

PKG_NAME:=lua-platform-info
PKG_VERSION:=1

include $(INCLUDE_DIR)/package.mk

define Package/lua-platform-info
  SECTION:=libs
  CATEGORY:=Libraries
  TITLE:=Lua hardware platform information library
  DEPENDS:=+lua +libplatforminfo
endef

define Package/lua-platform-info/install
	$(INSTALL_DIR) $(1)/usr/lib/lua
	$(CP) $(PKG_BUILD_DIR)/platform_info.so $(1)/usr/lib/lua/
endef

$(eval $(call BuildPackage,lua-platform-info))
