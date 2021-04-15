include $(TOPDIR)/rules.mk

PKG_NAME:=respondd-module-wifi
PKG_VERSION:=1
PKG_RELEASE:=1

PKG_LICENSE:=BSD-2-Clause

PKG_BUILD_DEPENDS := respondd

include $(INCLUDE_DIR)/package.mk

define Package/respondd-module-wifi/Default
  SECTION:=net
  CATEGORY:=Network
  TITLE:=Add wifi statistics to respondd
  DEPENDS:=+respondd +libnl-tiny
endef

TARGET_CFLAGS += -I$(STAGING_DIR)/usr/include/libnl-tiny

define Package/respondd-module-wifi
	$(call Package/respondd-module-wifi/Default)
endef

define Package/respondd-module-gluonwifi
	$(call Package/respondd-module-wifi/Default)
	VARIANT:=gluon
	TARGET_CFLAGS += -DGLUON
endef

define Package/respondd-module-wifi/install
	$(INSTALL_DIR) $(1)/usr/lib/respondd
	$(CP) $(PKG_BUILD_DIR)/respondd.so $(1)/usr/lib/respondd/wifi.so
endef

Package/respondd-module-gluonwifi/install = $(Package/respondd-module-wifi/install)

$(eval $(call BuildPackage,respondd-module-wifi))
$(eval $(call BuildPackage,respondd-module-gluonwifi))
