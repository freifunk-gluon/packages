include $(TOPDIR)/rules.mk

PKG_NAME:=lua-jsonc
PKG_VERSION:=1
CMAKE_INSTALL:=1

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/cmake.mk

define Package/lua-jsonc
  SECTION:=libs
  CATEGORY:=Libraries
  TITLE:=JSON parsing and formatting library
  DEPENDS:=+liblua +libjson-c
endef

define Package/lua-jsonc/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/* $(1)/usr/lib/
endef

$(eval $(call BuildPackage,lua-jsonc))
