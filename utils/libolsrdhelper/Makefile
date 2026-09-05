include $(TOPDIR)/rules.mk

PKG_NAME:=libolsrdhelper
PKG_VERSION:=1

PKG_LICENSE:=BSD-2-Clause

PKG_BUILD_DEPENDS:=olsrd-macd

include $(INCLUDE_DIR)/package.mk

define Package/libolsrdhelper
  SECTION:=libs
  CATEGORY:=Libraries
  TITLE:=OLSR utility library
  DEPENDS:= \
    +libjson-c \
    +libubox +libuclient \
    +liblua \
    +lua-jsonc
endef

define Package/libolsrdhelper/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/libolsrdhelper.so $(1)/usr/lib/
	$(INSTALL_DIR) $(1)/usr/lib/lua/gluon
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/olsrd.so $(1)/usr/lib/lua/gluon/
endef

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/lib
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/libolsrdhelper.so $(1)/usr/lib/
	$(INSTALL_DIR) $(1)/usr/include/libolsrdhelper
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/libolsrdhelper.h $(1)/usr/include/libolsrdhelper/
	$(INSTALL_DIR) $(1)/usr/lib/pkgconfig
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/libolsrdhelper.pc $(1)/usr/lib/pkgconfig/
endef

$(eval $(call BuildPackage,libolsrdhelper))
