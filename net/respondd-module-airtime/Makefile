include $(TOPDIR)/rules.mk

PKG_NAME:=respondd-module-airtime
PKG_VERSION:=1
PKG_RELEASE:=1

PKG_LICENSE:=BSD-2-Clause

PKG_BUILD_DIR := $(BUILD_DIR)/$(PKG_NAME)
PKG_BUILD_DEPENDS := respondd

include $(INCLUDE_DIR)/package.mk

define Package/respondd-module-airtime
  SECTION:=net
  CATEGORY:=Network
  TITLE:=Add airtime to respondd
  DEPENDS:=+respondd +libnl-tiny
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./src/* $(PKG_BUILD_DIR)/
endef

define Build/Configure
endef


TARGET_CFLAGS += -I$(STAGING_DIR)/usr/include/libnl-tiny

define Build/Compile
	CFLAGS="$(TARGET_CFLAGS)" CPPFLAGS="$(TARGET_CPPFLAGS)" $(MAKE) -C $(PKG_BUILD_DIR) $(TARGET_CONFIGURE_OPTS)
endef


define Package/respondd-module-airtime/install
	$(INSTALL_DIR) $(1)/usr/lib/respondd
	$(CP) $(PKG_BUILD_DIR)/respondd.so $(1)/usr/lib/respondd/airtime.so
endef

$(eval $(call BuildPackage,respondd-module-airtime))
