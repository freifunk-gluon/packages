include $(TOPDIR)/rules.mk

PKG_NAME:=ebtables-tiny
PKG_RELEASE:=1

PKG_LICENSE:=GPL-2.0

include $(INCLUDE_DIR)/package.mk

define Package/ebtables-tiny
  SECTION:=net
  CATEGORY:=Network
  SUBMENU:=Firewall
  CONFLICTS:=ebtables
  TITLE:=Ethernet bridge firewall administration utility (tiny)
endef

TARGET_CFLAGS += -fvisibility=hidden

define Package/ebtables-tiny/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/ebtables $(1)/usr/sbin/ebtables-tiny
endef

$(eval $(call BuildPackage,ebtables-tiny))
