include $(TOPDIR)/rules.mk

PKG_NAME:=lua-platform-info
PKG_VERSION:=1

PKG_BUILD_DIR := $(BUILD_DIR)/$(PKG_NAME)

include $(INCLUDE_DIR)/package.mk

define Package/lua-platform-info
  SECTION:=libs
  CATEGORY:=Libraries
  TITLE:=Lua hardware platform information library
  DEPENDS:=+lua +libplatforminfo
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./src/* $(PKG_BUILD_DIR)/
endef

define Build/Configure
endef

define Build/Compile
	CFLAGS="$(TARGET_CFLAGS)" \
	CPPFLAGS="$(TARGET_CPPFLAGS)" \
	$(MAKE) -C $(PKG_BUILD_DIR) $(TARGET_CONFIGURE_OPTS)
endef

define Package/lua-platform-info/install
	$(INSTALL_DIR) $(1)/usr/lib/lua
	$(CP) $(PKG_BUILD_DIR)/platform_info.so $(1)/usr/lib/lua/
endef

$(eval $(call BuildPackage,lua-platform-info))
