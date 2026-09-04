include $(TOPDIR)/rules.mk

PKG_NAME:=mmfd
PKG_SOURCE_DATE:=2019-11-23
PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL=https://github.com/freifunk-gluon/mmfd.git
PKG_SOURCE_VERSION:=a84bf1c26b577dc2cb36ec4f317bd4456e795d75

PKG_BUILD_DEPENDS += libjson-c

CMAKE_OPTIONS += -DCMAKE_BUILD_TYPE:STRING=MINSIZEREL
#CMAKE_OPTIONS += -DCMAKE_BUILD_TYPE:STRING=Debug

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/cmake.mk

define Package/mmfd
  SECTION:=net
  CATEGORY:=Network
  TITLE:=mesh multicast forwarding daemon
  DEPENDS:= +kmod-tun +libjson-c
endef

define Package/mmfd/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/src/mmfd $(1)/usr/sbin/
endef

$(eval $(call BuildPackage,mmfd))
