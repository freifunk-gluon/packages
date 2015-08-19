include $(TOPDIR)/rules.mk

PKG_NAME:=uradvd
PKG_VERSION:=1

PKG_BUILD_DIR := $(BUILD_DIR)/$(PKG_NAME)

include $(INCLUDE_DIR)/package.mk

define Package/uradvd
  SECTION:=net
  CATEGORY:=Network
  TITLE:=A tiny radvd
  DEPENDS:=+libc +kmod-ipv6 +librt
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

define Package/uradvd/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/uradvd $(1)/usr/sbin/
endef

$(eval $(call BuildPackage,uradvd))
