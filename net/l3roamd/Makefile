include $(TOPDIR)/rules.mk

PKG_NAME:=l3roamd
PKG_SOURCE_DATE:=2019-05-26
PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL=https://github.com/freifunk-gluon/l3roamd.git
PKG_SOURCE_VERSION=327a8fcdf34f6193b8144449a3ce3a96f78a5a24

CMAKE_OPTIONS += -DCMAKE_BUILD_TYPE:STRING=MINSIZEREL
#CMAKE_OPTIONS += -DCMAKE_BUILD_TYPE:STRING=Debug

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/cmake.mk

define Package/l3roamd
  SECTION:=net
  CATEGORY:=Network
  TITLE:=The layer 3 roaming daemon
  DEPENDS:=+libnl-tiny +kmod-tun +librt +libjson-c
endef

define Package/l3roamd/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/src/l3roamd $(1)/usr/sbin/
endef

$(eval $(call BuildPackage,l3roamd))
