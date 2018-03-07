include $(TOPDIR)/rules.mk

PKG_NAME:=uradvd
PKG_VERSION:=1

include $(INCLUDE_DIR)/package.mk

define Package/uradvd
  SECTION:=net
  CATEGORY:=Network
  TITLE:=A tiny radvd
  DEPENDS:=@IPV6 +librt
endef

define Package/uradvd/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/uradvd $(1)/usr/sbin/
endef

$(eval $(call BuildPackage,uradvd))
