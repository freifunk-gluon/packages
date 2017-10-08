include $(TOPDIR)/rules.mk

PKG_NAME:=mmfd
PKG_SOURCE_DATE:=2017-10-04
PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL=https://github.com/freifunk-gluon/mmfd.git
PKG_SOURCE_VERSION:=ba0fc88e2f1690e50341f25311ad35a72b1435e9

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/cmake.mk

define Package/mmfd
  SECTION:=net
  CATEGORY:=Network
  TITLE:=mesh multicast forwarding daemon
  DEPENDS:= +kmod-tun +babeld +libbabelhelper
endef

define Package/mmfd/install
	$(CP) ./files/* $(1)/
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/src/mmfd $(1)/usr/sbin/
endef

$(eval $(call BuildPackage,mmfd))
