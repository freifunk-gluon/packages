#
# Copyright (C) 2012 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=ecdsautils
PKG_VERSION:=0.3.2
PKG_RELEASE:=1
PKG_REV:=v$(PKG_VERSION)
PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=git://github.com/tcatm/$(PKG_NAME).git
PKG_SOURCE_VERSION:=$(PKG_REV)
PKG_SOURCE_SUBDIR:=$(PKG_NAME)-$(PKG_VERSION)
PKG_SOURCE_PROTO:=git

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
