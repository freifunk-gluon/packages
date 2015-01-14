include $(TOPDIR)/rules.mk

PKG_NAME:=gluon-collect-data
PKG_VERSION:=1
PKG_RELEASE:=1

PKG_BUILD_DIR := $(BUILD_DIR)/$(PKG_NAME)

include $(INCLUDE_DIR)/package.mk

define Package/gluon-collect-data
  SECTION:=gluon
  CATEGORY:=Gluon
  DEPENDS:=+gluon-core +luci-lib-json +lua-ethtool-stats
  TITLE:=Lua scripts collecting various information
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
endef

define Build/Configure
endef

define Build/Compile
endef

define Package/gluon-collect-data/install
	$(CP) ./files/* $(1)/
endef

$(eval $(call BuildPackage,gluon-collect-data))
