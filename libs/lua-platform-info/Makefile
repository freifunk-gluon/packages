include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/target.mk

PKG_NAME:=lua-platform-info
PKG_VERSION:=1

PKG_BUILD_DIR := $(BUILD_DIR)/$(PKG_NAME)

include $(INCLUDE_DIR)/package.mk

define Package/lua-platform-info
  SECTION:=libs
  CATEGORY:=Libraries
  TITLE:=Lua hardware platform information library
  DEPENDS:=+lua @(TARGET_ar71xx_generic||TARGET_ar71xx_nand||TARGET_mpc85xx_generic||TARGET_x86_generic||TARGET_x86_kvm_guest||TARGET_x86_64||TARGET_x86_xen_domu||TARGET_ramips_rt305x||TARGET_brcm2708_bcm2708||TARGET_brcm2708_bcm2709||TARGET_sunxi)
endef

define Package/lua-platform-info/description
	Lua library to obtain some generic information about the runtime platform
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
endef

define Build/Configure
endef

define Build/Compile
endef

define Package/lua-platform-info/install
	$(INSTALL_DIR) $(1)/usr/lib/lua
	$(INSTALL_DATA) ./files/$(BOARD)$(if $(SUBTARGET),/$(SUBTARGET))/platform_info.lua $(1)/usr/lib/lua/
endef

$(eval $(call BuildPackage,lua-platform-info))
