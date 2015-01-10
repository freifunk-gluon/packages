include $(TOPDIR)/rules.mk

PKG_NAME:=gluon-node-purpose
PKG_VERSION:=1
PKG_RELEASE:=1

PKG_BUILD_DIR := $(BUILD_DIR)/$(PKG_NAME)

include $(INCLUDE_DIR)/package.mk

define Package/gluon-node-purpose
  SECTION:=gluon
  CATEGORY:=Gluon
  TITLE:=Add /etc/config/gluon-node-purpose to uci
  DEPENDS:=+gluon-core +gluon-node-info
endef

define Package/gluon-node-purpose/description
	This package adds the section purpose to /etc/config/gluon-node-info.
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
endef

define Build/Configure
endef

define Build/Compile
endef

define Package/gluon-node-purpose/install
	$(CP) ./files/* $(1)/
endef

$(eval $(call BuildPackage,gluon-node-purpose))
