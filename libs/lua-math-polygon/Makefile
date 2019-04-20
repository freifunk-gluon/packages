include $(TOPDIR)/rules.mk

PKG_NAME:=lua-math-polygon
PKG_VERSION:=1

include $(INCLUDE_DIR)/package.mk

define Package/lua-math-polygon
	SECTION:=libs
	CATEGORY:=Libraries
	TITLE:=Polygon math library can also convert 2 point rectangles into polygons
endef

define Build/Compile
endef

define Package/lua-math-polygon/install
	$(INSTALL_DIR) $(1)/usr/lib/lua
	$(INSTALL_DATA) src/math-polygon.lua $(1)/usr/lib/lua/
endef

$(eval $(call BuildPackage,lua-math-polygon))
