include $(TOPDIR)/rules.mk

PKG_NAME:=libpacketmark
PKG_VERSION:=3

include $(INCLUDE_DIR)/package.mk

define Package/libpacketmark
  SECTION:=libs
  CATEGORY:=Libraries
  TITLE:=Set default packet mark via LD_PRELOAD
  DEPENDS:=
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./src/* $(PKG_BUILD_DIR)/
endef

define Build/Configure
endef

define Build/Compile
	CFLAGS="$(TARGET_CFLAGS)" CPPFLAGS="$(TARGET_CPPFLAGS)" $(MAKE) -C $(PKG_BUILD_DIR) $(TARGET_CONFIGURE_OPTS)
endef

define Package/libpacketmark/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_BUILD_DIR)/libpacketmark.so $(1)/usr/lib/
endef

$(eval $(call BuildPackage,libpacketmark))
