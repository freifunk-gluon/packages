include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/target.mk

PKG_NAME:=libplatforminfo
PKG_VERSION:=1
CMAKE_INSTALL:=1

PKG_LICENSE:=BSD-2-Clause

PKG_FLAGS:=nonshared

PKG_BUILD_DIR := $(KERNEL_BUILD_DIR)/$(PKG_NAME)

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/cmake.mk

define Package/libplatforminfo
  SECTION:=libs
  CATEGORY:=Libraries
  TITLE:=Platform information library
  DEPENDS:=@(TARGET_ar71xx_generic||TARGET_ar71xx_mikrotik||TARGET_ar71xx_nand||TARGET_mpc85xx_generic||TARGET_x86_generic||TARGET_x86_geode||TARGET_x86_kvm_guest||TARGET_x86_64||TARGET_x86_xen_domu||TARGET_ramips_mt7621||TARGET_ramips_mt7628||TARGET_ramips_rt305x||TARGET_brcm2708_bcm2708||TARGET_brcm2708_bcm2709||TARGET_sunxi||TARGET_mvebu)
endef

CMAKE_OPTIONS += \
	-DCMAKE_BUILD_TYPE:String="MINSIZEREL" \
	-DTARGET:STRING="$(BOARD)" \
	-DSUBTARGET:STRING="$(SUBTARGET)"



define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./src/* $(PKG_BUILD_DIR)/
endef

define Package/libplatforminfo/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/lib/libplatforminfo.so $(1)/usr/lib/
endef

$(eval $(call BuildPackage,libplatforminfo))
