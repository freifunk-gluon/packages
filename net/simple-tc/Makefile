include $(TOPDIR)/rules.mk

PKG_NAME:=simple-tc
PKG_VERSION:=1

include $(INCLUDE_DIR)/package.mk

define Package/simple-tc
  SECTION:=net
  CATEGORY:=Network
  TITLE:=Simple bandwidth limiting
  DEPENDS:=+kmod-sched +libnl-tiny
endef

TARGET_CFLAGS += -I$(STAGING_DIR)/usr/include/libnl-tiny

define Package/simple-tc/install
	$(CP) ./files/* $(1)/
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/simple-tc $(1)/usr/sbin/
endef

$(eval $(call BuildPackage,simple-tc))
