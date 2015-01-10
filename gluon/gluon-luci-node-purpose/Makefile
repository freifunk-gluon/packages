include $(TOPDIR)/rules.mk

PKG_NAME:=gluon-luci-node-purpose
PKG_VERSION:=0.1
PKG_RELEASE:=1

PKG_BUILD_DIR := $(BUILD_DIR)/$(PKG_NAME)

include $(INCLUDE_DIR)/package.mk

define Package/gluon-luci-node-purpose
  SECTION:=gluon
  CATEGORY:=Gluon
  DEPENDS:=+gluon-luci-admin +gluon-node-purpose
  TITLE:=UI for specifying node purpose
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
endef

define Build/Configure
endef

define Build/Compile
endef

define Package/gluon-luci-node-purpose/install
	$(CP) ./files/* $(1)/
endef

$(eval $(call BuildPackage,gluon-luci-node-purpose))
