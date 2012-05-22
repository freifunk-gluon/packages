# Copyright (C) 2012 Nils Schneider <nils at nilsschneider.net>
# This is free software, licensed under the Apache 2.0 license.

include $(TOPDIR)/rules.mk

PKG_NAME:=config-modee
PKG_RELEASE:=0.0.1

PKG_BUILD_DIR := $(BUILD_DIR)/$(PKG_NAME)

include $(INCLUDE_DIR)/package.mk

define Package/config-modee
  SECTION:=luci
  CATEGORY:=LuCI
  SUBMENU:=9. Freifunk
  TITLE:=Luci based config-modee for userfriendly setup of new meshnodes
  DEPENDS:=+firewall
endef

define Package/config-modee/description
	Luci based config-modee
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
endef

define Build/Configure
endef

define Build/Compile
endef

define Package/config-modee/install
	$(CP) ./files/* $(1)/
endef

$(eval $(call BuildPackage,config-modee))
