include $(TOPDIR)/rules.mk

PKG_NAME:=batman-adv-visdata
PKG_VERSION:=1

PKG_LICENSE:=BSD-2-Clause

PKG_BUILD_DIR := $(BUILD_DIR)/$(PKG_NAME)

include $(INCLUDE_DIR)/package.mk

define Package/batman-adv-visdata
  SECTION:=net
  CATEGORY:=Network
  DEPENDS:=
  TITLE:=Pulls the batman-adv originators and transtable_global tables out of the kernel periodically
endef


define Build/Prepare
        mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./src/* $(PKG_BUILD_DIR)/
endef

define Build/Configure
endef

define Build/Compile
	CFLAGS="$(TARGET_CFLAGS)" CPPFLAGS="$(TARGET_CPPFLAGS)" $(MAKE) -C $(PKG_BUILD_DIR) $(TARGET_CONFIGURE_OPTS)
endef

define Package/batman-adv-visdata/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/batman-adv-visdata $(1)/usr/sbin/

	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) ./files/batman-adv-visdata.init $(1)/etc/init.d/batman-adv-visdata
endef

$(eval $(call BuildPackage,batman-adv-visdata))
