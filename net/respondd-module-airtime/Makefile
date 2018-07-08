include $(TOPDIR)/rules.mk

PKG_NAME:=respondd-module-airtime
PKG_VERSION:=1
PKG_RELEASE:=2

PKG_LICENSE:=BSD-2-Clause

PKG_BUILD_DEPENDS := respondd

include $(INCLUDE_DIR)/package.mk

define Package/respondd-module-airtime
  SECTION:=net
  CATEGORY:=Network
  TITLE:=Add airtime to respondd
  DEPENDS:=+respondd +libnl-tiny
endef

TARGET_CFLAGS += -I$(STAGING_DIR)/usr/include/libnl-tiny

define Package/respondd-module-airtime/install
	$(INSTALL_DIR) $(1)/usr/lib/respondd
	$(CP) $(PKG_BUILD_DIR)/respondd.so $(1)/usr/lib/respondd/airtime.so
endef

$(eval $(call BuildPackage,respondd-module-airtime))
