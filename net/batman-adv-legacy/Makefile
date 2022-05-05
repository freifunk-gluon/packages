#
# Copyright (C) 2010 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.

include $(TOPDIR)/rules.mk

PKG_NAME:=batman-adv-legacy

PKG_VERSION:=2019-04-22
PKG_RELEASE:=1

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL:=https://github.com/freifunk-gluon/batman-adv-legacy.git
PKG_SOURCE_SUBDIR:=$(PKG_NAME)-$(PKG_VERSION)
PKG_SOURCE_VERSION:=ea42aed32284121e90857089cd741e237d34cdea
PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION)-$(PKG_SOURCE_VERSION).tar.gz

include $(INCLUDE_DIR)/kernel.mk
include $(INCLUDE_DIR)/package.mk

define KernelPackage/batman-adv-legacy
  URL:=http://www.open-mesh.org/
  MAINTAINER:=Matthias Schiffer <mschiffer@universe-factory.net>, Linus Lüssing <linus.luessing@c0d3.blue>
  SUBMENU:=Network Support
  DEPENDS:=+kmod-lib-crc16 +kmod-crypto-crc32c +kmod-lib-crc32c +libc
  TITLE:=B.A.T.M.A.N. Adv Legacy
  FILES:=$(PKG_BUILD_DIR)/batman-adv-legacy.$(LINUX_KMOD_SUFFIX)
endef

define KernelPackage/batman-adv-legacy/description
B.A.T.M.A.N. advanced is a kernel module which allows to
build layer 2 mesh networks.
endef

define KernelPackage/batman-adv-legacy/config
	source "$(SOURCE)/Config.in"
endef

MAKE_FLAGS += \
	CROSS_COMPILE="$(TARGET_CROSS)" \
	KERNELPATH="$(LINUX_DIR)" \
	ARCH="$(LINUX_KARCH)" \
	PATH="$(TARGET_PATH)" \
	SUBDIRS="$(PKG_BUILD_DIR)" \
	PWD="$(PKG_BUILD_DIR)" \
	LINUX_VERSION="$(LINUX_VERSION)" \
	CONFIG_BATMAN_ADV_DEBUG=$(if $(CONFIG_KMOD_BATMAN_ADV_LEGACY_DEBUG_LOG),y,n) \
	CONFIG_BATMAN_ADV_BLA=$(if $(CONFIG_KMOD_BATMAN_ADV_LEGACY_BLA),y,n) \
	CONFIG_BATMAN_ADV_DAT=$(if $(CONFIG_KMOD_BATMAN_ADV_LEGACY_DAT),y,n) \
	CONFIG_BATMAN_ADV_NC=$(if $(CONFIG_KMOD_BATMAN_ADV_LEGACY_NC),y,n) \
	REVISION="" all

define Build/Prepare
	$(call Build/Prepare/Default)
	$(SED) '/#define _NET_BATMAN_ADV_MAIN_H_/a\#undef CONFIG_MODULE_STRIPPED' \
		$(PKG_BUILD_DIR)/main.h
endef

define Build/Compile
	$(call Build/Compile/Default,)
	mv $(PKG_BUILD_DIR)/batman-adv.$(LINUX_KMOD_SUFFIX) $(PKG_BUILD_DIR)/batman-adv-legacy.$(LINUX_KMOD_SUFFIX)
endef

define KernelPackage/batman-adv-legacy/install
	$(INSTALL_DIR) $(1)/etc/config $(1)/etc/hotplug.d/net $(1)/etc/hotplug.d/iface $(1)/lib/batman-adv-legacy $(1)/usr/sbin $(1)/lib/netifd/proto
	$(INSTALL_DATA) ./files/etc/config/batman-adv-legacy $(1)/etc/config
	$(INSTALL_DATA) ./files/lib/batman-adv-legacy/config.sh $(1)/lib/batman-adv-legacy
	$(INSTALL_BIN) ./files/etc/hotplug.d/net/99-batman-adv-legacy $(1)/etc/hotplug.d/net
	$(INSTALL_BIN) ./files/lib/netifd/proto/batadv_legacy.sh $(1)/lib/netifd/proto
endef

$(eval $(call KernelPackage,batman-adv-legacy))
