include $(TOPDIR)/rules.mk

PKG_NAME:=mmfd
PKG_SOURCE_DATE:=2017-02-23
PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL=https://github.com/freifunk-gluon/mmfd.git
PKG_SOURCE_VERSION:=9f502ace1fc0353e025387f3f531c6ee4f35c00e

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/cmake.mk

define Package/mmfd
  SECTION:=net
  CATEGORY:=Network
  TITLE:=mesh multicast forwarding daemon
  DEPENDS:= +kmod-tun +babeld
endef

define Package/mmfd/install
	$(CP) ./files/* $(1)/
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/src/mmfd $(1)/usr/sbin/
endef

$(eval $(call BuildPackage,mmfd))
