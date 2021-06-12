include $(TOPDIR)/rules.mk

PKG_NAME:=lua-jsonc
PKG_VERSION:=1
CMAKE_INSTALL:=1

HOST_BUILD_DEPENDS:=lua/host libjson-c/host
HOST_UNPACK:=true

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/host-build.mk
include $(INCLUDE_DIR)/cmake.mk

define Package/lua-jsonc
  SECTION:=libs
  CATEGORY:=Libraries
  TITLE:=JSON parsing and formatting library
  DEPENDS:=+liblua +libjson-c
endef

define Package/lua-jsonc/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/* $(1)/usr/lib/
endef

# For libjson-c
HOST_LDFLAGS += -Wl,-rpath,$(STAGING_DIR_HOST)/lib

define Host/Install
	$(INSTALL_DIR) $(1)/usr/lib/lua/5.1
	$(INSTALL_BIN) $(HOST_BUILD_DIR)/liblua-jsonc.so $(1)/lib/lua/5.1/jsonc.so
endef

$(eval $(call BuildPackage,lua-jsonc))
$(eval $(call HostBuild))
