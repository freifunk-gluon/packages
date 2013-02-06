#
# Copyright (C) 2012 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=ecdsautils
PKG_VERSION:=0.2
PKG_RELEASE:=1

PKG_SOURCE:=v$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://github.com/tcatm/ecdsautils/archive/
PKG_MD5SUM:=bd170e48f045d62b0b4d22c26c1a3f2a

PKG_BUILD_DEPENDS:=libuecc

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/cmake.mk

define Package/ecdsautils
  SECTION:=utils
  CATEGORY:=Utilities
  DEPENDS:=
  TITLE:=ECDSA Utilities
  URL:=https://github.com/tcatm/ecdsautils
endef

CMAKE_OPTIONS += \
	-DCMAKE_BUILD_TYPE:String="MINSIZEREL" \


define Package/ecdsautils/description
 Utilities to sign and verify checksums using ECDSA.
endef

define Package/ecdsautils/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/ecdsakeygen $(1)/usr/bin/
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/ecdsasign $(1)/usr/bin/
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/ecdsaverify $(1)/usr/bin/
endef

$(eval $(call BuildPackage,ecdsautils))
