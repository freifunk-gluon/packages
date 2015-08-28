include $(TOPDIR)/rules.mk

PKG_NAME:=node
PKG_VERSION:=0.12.7
PKG_RELEASE:=1
PKG_MD5SUM:=5523ec4347d7fe6b0f6dda1d1c7799d5

PKG_SOURCE:=node-v$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://nodejs.org/dist/v$(PKG_VERSION)/
PKG_HOST_ONLY=1

PKG_MAINTAINER:=Nils Schneider <nils@nilsschneider.net>
PKG_LICENSE:=MIT
PKG_LICENSE_FILES:=LICENSE

HOST_BUILD_DIR:=$(BUILD_DIR)/node-v$(PKG_VERSION)
HOST_BUILD_DEPENDS:=python/host

include $(INCLUDE_DIR)/host-build.mk
include $(INCLUDE_DIR)/package.mk

define Package/node
  SECTION:=lang
  CATEGORY:=Languages
  TITLE:=Evented I/O for V8 javascript
  BUILDONLY:=1
  URL:=https://nodejs.org
endef

define Host/Configure
  cd $(HOST_BUILD_DIR) && ./configure --prefix=$(STAGING_DIR_HOST)
endef

define Host/Install
  $(MAKE) -C $(HOST_BUILD_DIR) install
endef

$(eval $(call HostBuild))
$(eval $(call BuildPackage,node))
