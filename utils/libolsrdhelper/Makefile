include $(TOPDIR)/rules.mk

PKG_NAME:=libolsrdhelper
PKG_VERSION=1

PKG_BUILD_DEPENDS += libjson-c

define Package/libolsrdhelper
  SECTION:=libs
  CATEGORY:=Libraries
  TITLE:=OLSR utility library
  DEPENDS:= \
    +libgluonutil \
    +libjson-c \
    +libubox +libuclient \
    +liblua \
    +lua-jsonc
endef

define Package/gluon-mesh-olsrd/install
	$(INSTALL_DIR) $(1)/usr/lib/lua/gluon
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/olsrd.so $(1)/usr/lib/lua/gluon/
	$(INSTALL_DIR) $(1)/usr/lib/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/libolsrdhelper.so $(1)/usr/lib/
endef

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/lib/lua/gluon
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/olsrd.so $(1)/usr/lib/lua/gluon/
	$(INSTALL_DIR) $(1)/usr/lib/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/libolsrdhelper.so $(1)/usr/lib/

	$(INSTALL_DIR) $(1)/usr/include/gluon-mesh-olsrd
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/lib/libolsrdhelper.h $(1)/usr/include/gluon-mesh-olsrd/
	$(INSTALL_DIR) $(1)/usr/lib/pkgconfig/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/lib/libolsrdhelper.pc $(1)/usr/lib/pkgconfig/
endef


$(eval $(call BuildPackageGluon,gluon-mesh-olsrd))
