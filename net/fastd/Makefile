#
# Copyright (C) 2012 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=fastd
PKG_VERSION:=0.5-rc2
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.bz2
PKG_SOURCE_URL:=https://projects.universe-factory.net/attachments/download/32
PKG_MD5SUM:=74b2ab08f90303f8ae1937e99c734134

PKG_CONFIG_DEPENDS:=CONFIG_FASTD_DISABLE_METHOD_XSALSA20_POLY1305 CONFIG_FASTD_DISABLE_METHOD_AES128_GCM
PKG_BUILD_DEPENDS:=nacl libuecc

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/cmake.mk

define Package/fastd
  SECTION:=net
  CATEGORY:=Network
  DEPENDS:=+kmod-tun +librt +libpthread
  TITLE:=Fast and Secure Tunneling Daemon
  URL:=https://projects.universe-factory.net/projects/fastd
  SUBMENU:=VPN
endef

define Package/fastd/config
  source "$(SOURCE)/Config.in"
endef

CMAKE_OPTIONS += \
	-DCMAKE_BUILD_TYPE:String="MINSIZEREL" \


ifeq ($(CONFIG_FASTD_DISABLE_METHOD_XSALSA20_POLY1305),y)
CMAKE_OPTIONS += \
	-DWITH_METHOD_XSALSA20_POLY1305:BOOL=FALSE
else
CMAKE_OPTIONS += \
	-DWITH_METHOD_XSALSA20_POLY1305:BOOL=TRUE
endif

ifeq ($(CONFIG_FASTD_DISABLE_METHOD_AES128_GCM),y)
CMAKE_OPTIONS += \
	-DWITH_METHOD_AES128_GCM:BOOL=FALSE
else
CMAKE_OPTIONS += \
	-DWITH_METHOD_AES128_GCM:BOOL=TRUE
endif

define Package/fastd/description
 Fast and secure tunneling daemon, which is optimized on small code size and few dependencies
endef

define Package/fastd/conffiles
/etc/config/fastd
endef

define Package/fastd/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/sbin/fastd $(1)/usr/sbin/

	$(INSTALL_DIR) $(1)/etc/init.d/
	$(INSTALL_BIN) files/fastd.init $(1)/etc/init.d/fastd
	$(INSTALL_DIR) $(1)/etc/config
	$(INSTALL_CONF) files/fastd.config $(1)/etc/config/fastd
	$(INSTALL_DIR) $(1)/etc/fastd
	$(INSTALL_DIR) $(1)/lib/upgrade/keep.d
	$(INSTALL_DATA) files/fastd.upgrade $(1)/lib/upgrade/keep.d/fastd
endef

$(eval $(call BuildPackage,fastd))
