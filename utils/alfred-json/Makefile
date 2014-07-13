#
# Copyright (C) 2012 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=alfred-json
PKG_VERSION:=0.2
PKG_RELEASE:=2
PKG_REV:=v$(PKG_VERSION)

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=git://github.com/tcatm/$(PKG_NAME).git
PKG_SOURCE_VERSION:=$(PKG_REV)
PKG_SOURCE_SUBDIR:=$(PKG_NAME)-$(PKG_VERSION)
PKG_SOURCE_PROTO:=git

include $(INCLUDE_DIR)/package.mk

define Package/alfred-json
  SECTION:=utils
  CATEGORY:=Utilities
  DEPENDS:=+jansson +libc +librt +zlib
  TITLE:=ALFRED JSON client
  URL:=https://github.com/tcatm/alfred-json
endef

define Package/alfred-json/install
	$(INSTALL_DIR) $(1)/usr/bin
	cp -fpR  $(PKG_BUILD_DIR)/alfred-json $(1)/usr/bin/
endef

$(eval $(call BuildPackage,alfred-json))
