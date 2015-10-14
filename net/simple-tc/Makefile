include $(TOPDIR)/rules.mk

PKG_NAME:=simple-tc
PKG_VERSION:=1

PKG_BUILD_DIR := $(BUILD_DIR)/$(PKG_NAME)

include $(INCLUDE_DIR)/package.mk

define Package/simple-tc
  SECTION:=net
  CATEGORY:=Network
  TITLE:=Simple bandwidth limiting
  DEPENDS:=+kmod-sched +libnl-tiny
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

define Package/simple-tc/install
	$(CP) ./files/* $(1)/
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/simple-tc $(1)/usr/sbin/
endef

$(eval $(call BuildPackage,simple-tc))
