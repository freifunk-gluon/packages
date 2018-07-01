include $(TOPDIR)/rules.mk

PKG_NAME:=respondd-module-wifisettings
PKG_VERSION:=1
PKG_RELEASE:=1

PKG_LICENSE:=BSD-2-Clause

PKG_BUILD_DEPENDS := respondd

include $(INCLUDE_DIR)/package.mk

define Package/respondd-module-wifisettings
  SECTION:=net
  CATEGORY:=Network
  TITLE:=Add wifisettings to respondd
  DEPENDS:=+respondd +libuci
endef

define Package/respondd-module-wifisettings/install
	$(INSTALL_DIR) $(1)/usr/lib/respondd
	$(CP) $(PKG_BUILD_DIR)/respondd.so $(1)/usr/lib/respondd/wifi-settings.so
endef

$(eval $(call BuildPackage,respondd-module-wifisettings))
