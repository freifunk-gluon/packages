include $(TOPDIR)/rules.mk

PKG_NAME:=olsrd-macd
PKG_VERSION:=1

PKG_LICENSE:=BSD-2-Clause

include $(INCLUDE_DIR)/package.mk

define Package/olsrd-macd
  SECTION:=net
  CATEGORY:=Network
  TITLE:=MAC lookup for OLSR neighbours
  DEPENDS:=+libjson-c +libubox
endef

define Package/olsrd-macd/description
 Remembers the MAC each OLSR neighbour address was last seen with and
 answers over /var/run/olsr-macd.sock. olsrd itself only sees UDP.
endef

define Package/olsrd-macd/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/olsr-macd $(1)/usr/bin/
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) ./files/etc/init.d/olsr-macd $(1)/etc/init.d/
endef

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include/olsr-macd
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/olsr-macd.h $(1)/usr/include/olsr-macd/
	$(INSTALL_DIR) $(1)/usr/lib/pkgconfig
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/olsr-macd.pc $(1)/usr/lib/pkgconfig/
endef

$(eval $(call BuildPackage,olsrd-macd))
