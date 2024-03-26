# SPDX-License-Identifier: MIT
# Copyright (C) 2023 Linus Lüssing <linus.luessing@c0d3.blue>

include $(TOPDIR)/rules.mk

PKG_NAME:=brmldproxy
PKG_RELEASE:=1

PKG_SOURCE_PROTO:=git
PKG_SOURCE_DATE:=2023-12-31
PKG_SOURCE_URL=https://github.com/T-X/brmldproxy.git
PKG_SOURCE_VERSION:=4d7fdb1a5c6e726b4c1930ad411d5571e09fa2f0
PKG_MIRROR_HASH:=1541eeaf6ae2fb4390448f02c5486da708cfa4d6200be77f884b47a2c86a7d06

PKG_MAINTAINER:=Linus Lüssing <linus.luessing@c0d3.blue>
PKG_LICENSE:=GPL-2.0-or-later
PKG_LICENSE_FILES:=LICENSE

include $(INCLUDE_DIR)/package.mk

define Package/brmldproxy
  SECTION:=net
  CATEGORY:=Network
  TITLE:=Bridge MLD Proxy
  DEPENDS:=+tc
endef

define Package/brmldproxy/description
  A userspace controlled MLD proxy implementation for a Linux bridge.
  The bridge itself will appear as a single multicast listening host
  to any MLD querier on a configured proxy port, acting in deputy
  for any other multicast listener behind adjacent bridge ports.
  This potentially reduces MLD report overhead.
  brmldproxy further allows to filter out specific multicast groups
  and bridge ports from its combined MLD report.
endef

define Package/brmldproxy/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/brmldproxy $(1)/usr/sbin/
	$(CP) ./files/* $(1)/
endef

$(eval $(call BuildPackage,brmldproxy))
