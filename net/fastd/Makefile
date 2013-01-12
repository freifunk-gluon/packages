#
# Copyright (C) 2012 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=fastd
PKG_VERSION:=7
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.xz
PKG_SOURCE_URL:=https://projects.universe-factory.net/attachments/download/43
PKG_MD5SUM:=17dd6049d072f24dba9d959ac9edbc16

PKG_CONFIG_DEPENDS:=\
	CONFIG_FASTD_ENABLE_METHOD_XSALSA20_POLY1305 \
	CONFIG_FASTD_ENABLE_METHOD_AES128_GCM \
	CONFIG_FASTD_ENABLE_CRYPTO_AES128CTR_NACL \
	CONFIG_FASTD_ENABLE_CRYPTO_AES128CTR_LINUX \
	CONFIG_FASTD_ENABLE_CRYPTO_GHASH_BUILTIN \
	CONFIG_FASTD_ENABLE_CRYPTO_GHASH_LINUX

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
	-DWITH_CAPABILITIES=FALSE

ifeq ($(CONFIG_FASTD_ENABLE_METHOD_XSALSA20_POLY1305),y)
CMAKE_OPTIONS += \
	-DWITH_METHOD_XSALSA20_POLY1305:BOOL=TRUE
else
CMAKE_OPTIONS += \
	-DWITH_METHOD_XSALSA20_POLY1305:BOOL=FALSE
endif

ifeq ($(CONFIG_FASTD_ENABLE_METHOD_AES128_GCM),y)
CMAKE_OPTIONS += \
	-DWITH_METHOD_AES128_GCM:BOOL=TRUE
else
CMAKE_OPTIONS += \
	-DWITH_METHOD_AES128_GCM:BOOL=FALSE
endif

ifeq ($(CONFIG_FASTD_ENABLE_CRYPTO_AES128CTR_NACL),y)
CMAKE_OPTIONS += \
	-DWITH_CRYPTO_AES128CTR_NACL:BOOL=TRUE
else
CMAKE_OPTIONS += \
	-DWITH_CRYPTO_AES128CTR_NACL:BOOL=FALSE
endif

ifeq ($(CONFIG_FASTD_ENABLE_CRYPTO_AES128CTR_LINUX),y)
CMAKE_OPTIONS += \
	-DWITH_CRYPTO_AES128CTR_LINUX:BOOL=TRUE
else
CMAKE_OPTIONS += \
	-DWITH_CRYPTO_AES128CTR_LINUX:BOOL=FALSE
endif

ifeq ($(CONFIG_FASTD_ENABLE_CRYPTO_GHASH_BUILTIN),y)
CMAKE_OPTIONS += \
	-DWITH_CRYPTO_GHASH_BUILTIN:BOOL=TRUE
else
CMAKE_OPTIONS += \
	-DWITH_CRYPTO_GHASH_BUILTIN:BOOL=FALSE
endif

ifeq ($(CONFIG_FASTD_ENABLE_CRYPTO_GHASH_LINUX),y)
CMAKE_OPTIONS += \
	-DWITH_CRYPTO_GHASH_LINUX:BOOL=TRUE
else
CMAKE_OPTIONS += \
	-DWITH_CRYPTO_GHASH_LINUX:BOOL=FALSE
endif


define Package/fastd/description
 Fast and secure tunneling daemon, which is optimized on small code size and few dependencies
endef

define Package/fastd/conffiles
/etc/config/fastd
endef

define Package/fastd/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/fastd $(1)/usr/bin/

	$(INSTALL_DIR) $(1)/etc/init.d/
	$(INSTALL_BIN) files/fastd.init $(1)/etc/init.d/fastd
	$(INSTALL_DIR) $(1)/etc/config
	$(INSTALL_CONF) files/fastd.config $(1)/etc/config/fastd
	$(INSTALL_DIR) $(1)/etc/fastd
	$(INSTALL_DIR) $(1)/lib/upgrade/keep.d
	$(INSTALL_DATA) files/fastd.upgrade $(1)/lib/upgrade/keep.d/fastd
endef

$(eval $(call BuildPackage,fastd))
