include $(TOPDIR)/rules.mk

PKG_NAME:=lua-hash
PKG_VERSION:=1

PKG_BUILD_DIR := $(BUILD_DIR)/$(PKG_NAME)

include $(INCLUDE_DIR)/package.mk

define Package/lua-hash
  SECTION:=libs
  CATEGORY:=Libraries
  TITLE:=Lua hash library
  DEPENDS:=+lua +libubox
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

define Package/lua-hash/install
	$(INSTALL_DIR) $(1)/usr/lib/lua
	$(CP) $(PKG_BUILD_DIR)/hash.so $(1)/usr/lib/lua/
endef

$(eval $(call BuildPackage,lua-hash))
