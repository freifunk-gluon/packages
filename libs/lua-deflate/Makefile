include $(TOPDIR)/rules.mk

PKG_NAME:=lua-deflate
PKG_VERSION:=1

PKG_LICENSE:=BSD-2-Clause

PKG_BUILD_DIR := $(BUILD_DIR)/$(PKG_NAME)

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/cmake.mk

define Package/lua-deflate
  SECTION:=libs
  CATEGORY:=Libraries
  TITLE:=A minimal deflate compression library for Lua
  DEPENDS:=+lua
endef

CMAKE_OPTIONS += -DCMAKE_BUILD_TYPE:STRING=MINSIZEREL

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./src/* $(PKG_BUILD_DIR)/
endef

define Package/lua-deflate/install
	$(INSTALL_DIR) $(1)/usr/lib/lua
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/lua/deflate.so $(1)/usr/lib/lua/
endef

$(eval $(call BuildPackage,lua-deflate))
