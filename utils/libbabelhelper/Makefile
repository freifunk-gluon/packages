include $(TOPDIR)/rules.mk

PKG_NAME:=libbabelhelper
PKG_RELEASE:=1
PKG_VERSION:=1

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL=https://github.com/christf/libbabelhelper.git
PKG_SOURCE_VERSION:=d1b8b6f6cdf51aacedcab0ee5d9b18e21fb264ff

CMAKE_INSTALL:=1

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/cmake.mk
CMAKE_OPTIONS += -DCMAKE_BUILD_TYPE:String="MINSIZEREL"

define Package/libbabelhelper
  SECTION:=libs
  CATEGORY:=Libraries
  TITLE:=Babel utility library
endef

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include
	$(CP) $(PKG_INSTALL_DIR)/usr/include/libbabelhelper-$(PKG_VERSION) $(1)/usr/include/
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
