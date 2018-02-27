include $(TOPDIR)/rules.mk

PKG_NAME:=lua-geolocate
PKG_VERSION:=1

include $(INCLUDE_DIR)/package.mk

define Package/lua-geolocate
	SECTION:=libs
	CATEGORY:=Libraries
	TITLE:=WLAN-based geolocation librari based on openwifi.su
	DEPENDS:=+libubus-lua +libiwinfo-lua +lua-jsonc
endef

define Build/Compile
endef

define Package/lua-geolocate/install
	$(INSTALL_DIR) $(1)/usr/lib/lua
	$(INSTALL_DATA) src/geolocate.lua $(1)/usr/lib/lua/
endef

$(eval $(call BuildPackage,lua-geolocate))
