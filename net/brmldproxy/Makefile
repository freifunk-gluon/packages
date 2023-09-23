# SPDX-License-Identifier: MIT
# Copyright (C) 2023 Linus Lüssing <linus.luessing@c0d3.blue>

include $(TOPDIR)/rules.mk

PKG_NAME:=brmldproxy
PKG_RELEASE:=1

PKG_SOURCE_PROTO:=git
PKG_SOURCE_DATE:=2025-01-06
PKG_SOURCE_URL=https://github.com/T-X/brmldproxy.git
PKG_SOURCE_VERSION:=2b9ec1b64f7bffbbe01271894d53f1b371f48dfa
PKG_MIRROR_HASH:=3db998649cc99ec0e64d39f7b06319a1b6e2b0f50e68e3cbb8f48023c31f0e3d

PKG_MAINTAINER:=Linus Lüssing <linus.luessing@c0d3.blue>
PKG_LICENSE:=GPL-2.0-or-later
PKG_LICENSE_FILES:=LICENSE

include $(INCLUDE_DIR)/package.mk

define Package/$(PKG_NAME)
  SECTION:=net
  CATEGORY:=Network
  TITLE:=Bridge MLD Proxy
  DEPENDS:=+tc
endef

define Package/$(PKG_NAME)/description
  A userspace controlled MLD proxy implementation for a Linux bridge.
  The bridge itself will appear as a single multicast listening host
  to any MLD querier on a configured proxy port, acting in deputy
  for any other multicast listener behind adjacent bridge ports.
  This potentially reduces MLD report overhead.
  brmldproxy further allows to filter out specific multicast groups
  and bridge ports from its combined MLD report.
endef

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/brmldproxy $(1)/usr/sbin/
	$(CP) ./files/* $(1)/
endef

$(eval $(call BuildPackage,$(PKG_NAME)))
