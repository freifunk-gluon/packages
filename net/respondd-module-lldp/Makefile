include $(TOPDIR)/rules.mk

PKG_NAME:=respondd-module-lldp
PKG_VERSION:=1
PKG_RELEASE:=1

include $(INCLUDE_DIR)/package.mk

define Package/respondd-module-lldp
  SECTION:=gluon
  CATEGORY:=Gluon
  TITLE:=adds lldp usage neighbours provider to respondd
  DEPENDS:=+respondd +lldpd
endef

define Package/respondd-module-lldp/install
	$(INSTALL_DIR) $(1)/usr/lib/respondd
	$(CP) $(PKG_BUILD_DIR)/respondd.so $(1)/usr/lib/respondd/lldp.so
endef

$(eval $(call BuildPackage,respondd-module-lldp))
