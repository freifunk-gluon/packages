include $(TOPDIR)/rules.mk

PKG_NAME:=gluon-announced
PKG_VERSION:=2
PKG_RELEASE:=2

PKG_BUILD_DIR := $(BUILD_DIR)/$(PKG_NAME)

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/cmake.mk

define Package/gluon-announced
  SECTION:=gluon
  CATEGORY:=Gluon
  TITLE:=announced support
  DEPENDS:=+liblua +gluon-announce
endef

define Package/gluon-announced/description
	Gluon community wifi mesh firmware framework: announced support
endef

define Package/gluon-announced/install
	$(CP) ./files/* $(1)/
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/gluon-announced $(1)/usr/bin/
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./src/* $(PKG_BUILD_DIR)/
endef

$(eval $(call BuildPackage,gluon-announced))
