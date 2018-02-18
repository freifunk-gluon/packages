include $(TOPDIR)/rules.mk

PKG_NAME:=autoupdater
PKG_VERSION:=4

PKG_BUILD_DIR := $(BUILD_DIR)/$(PKG_NAME)

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/cmake.mk

define Package/autoupdater
  SECTION:=admin
  CATEGORY:=Administration
  DEPENDS:=+libuclient +libecdsautil +libplatforminfo +libuci
  TITLE:=Automatically update firmware
endef

define Package/autoupdater/conffiles
/etc/config/autoupdater
endef


CMAKE_OPTIONS += \
    -DCMAKE_BUILD_TYPE:String="MINSIZEREL"

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./src/* $(PKG_BUILD_DIR)/
endef

define Package/autoupdater/install
	$(CP) ./files/* $(1)/
	$(INSTALL_DIR) $(1)/usr/sbin/
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/sbin/autoupdater $(1)/usr/sbin/
endef

$(eval $(call BuildPackage,autoupdater))
