include $(TOPDIR)/rules.mk

PKG_NAME:=luasrcdiet
PKG_VERSION:=1.0.0
PKG_RELEASE:=1

PKG_MAINTAINER:=Matthias Schiffer <mschiffer@universe-factory.net>
PKG_LICENSE:=MIT
PKG_LICENSE_FILES:=COPYRIGHT

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://github.com/jirutka/luasrcdiet/archive/
PKG_SOURCE_URL_FILE:=v$(PKG_VERSION).tar.gz
PKG_HASH:=48162e63e77d009f5848f18a5cabffbdfc867d0e5e73c6d407f6af5d6880151b

HOST_BUILD_DEPENDS:=lua/host

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/host-build.mk

define Package/luasrcdiet
  SUBMENU:=Lua
  SECTION:=lang
  CATEGORY:=Languages
  TITLE:=luasrcdiet
  URL:=https://github.com/jirutka/luasrcdiet
  DEPENDS:=+lua
  PKGARCH:=all
endef

define Package/luasrcdiet/description
Compresses Lua source code by removing unnecessary characters
endef

define Build/Compile
endef

define Package/luasrcdiet/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/bin/luasrcdiet $(1)/usr/bin/luasrcdiet

	$(INSTALL_DIR) $(1)/usr/lib/lua
	$(CP) $(PKG_BUILD_DIR)/luasrcdiet $(1)/usr/lib/lua/
endef

define Host/Compile
endef

define Host/Install
	$(INSTALL_DIR) $(1)/bin
	$(INSTALL_BIN) $(HOST_BUILD_DIR)/bin/luasrcdiet $(1)/bin/

	$(INSTALL_DIR) $(1)/lib/lua/5.1
	$(CP) $(HOST_BUILD_DIR)/luasrcdiet $(1)/lib/lua/5.1/
endef

$(eval $(call HostBuild))
$(eval $(call BuildPackage,luasrcdiet))
