include $(TOPDIR)/rules.mk

PKG_NAME:=mmfd
PKG_SOURCE_DATE:=2017-02-15
PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL=https://github.com/freifunk-gluon/mmfd.git
PKG_SOURCE_SUBDIR:=$(PKG_NAME)-$(PKG_VERSION)
PKG_SOURCE_VERSION:=2aa515ec81efdbeccfd7543f0ee63232da09d399
PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION)-$(PKG_SOURCE_VERSION).tar.gz

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/cmake.mk

define Package/mmfd
  SECTION:=net
  CATEGORY:=Network
  TITLE:=mesh multicast forwarding daemon
  DEPENDS:= +kmod-tun
endef

define Package/mmfd/install
	$(CP) ./files/* $(1)/
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/src/mmfd $(1)/usr/sbin/
endef

$(eval $(call BuildPackage,mmfd))
