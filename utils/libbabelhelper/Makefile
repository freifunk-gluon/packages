include $(TOPDIR)/rules.mk

PKG_NAME:=libbabelhelper

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL:=https://github.com/christf/libbabelhelper.git
PKG_SOURCE_DATE:=2017-12-09
PKG_SOURCE_VERSION:=9656208866f371ffe6f1f0fb38d4f915eb4cf463


CMAKE_INSTALL:=1

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/cmake.mk

define Package/libbabelhelper
  SECTION:=libs
  CATEGORY:=Libraries
  TITLE:=Babel utility library
endef

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include
	$(CP) $(PKG_INSTALL_DIR)/usr/include/libbabelhelper-* $(1)/usr/include/
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libbabelhelper.so* $(1)/usr/lib/
	$(INSTALL_DIR) $(1)/usr/lib/pkgconfig
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/pkgconfig/libbabelhelper.pc $(1)/usr/lib/pkgconfig/
endef


define Package/libbabelhelper/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/src/libbabelhelper.so* $(1)/usr/lib/
endef

$(eval $(call BuildPackage,libbabelhelper))
