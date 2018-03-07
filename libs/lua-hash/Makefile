include $(TOPDIR)/rules.mk

PKG_NAME:=lua-hash
PKG_VERSION:=1

include $(INCLUDE_DIR)/package.mk

define Package/lua-hash
  SECTION:=libs
  CATEGORY:=Libraries
  TITLE:=Lua hash library
  DEPENDS:=+lua +libubox
endef

define Package/lua-hash/install
	$(INSTALL_DIR) $(1)/usr/lib/lua
	$(CP) $(PKG_BUILD_DIR)/hash.so $(1)/usr/lib/lua/
endef

$(eval $(call BuildPackage,lua-hash))
