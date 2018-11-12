include $(TOPDIR)/rules.mk

PKG_NAME:=libplatforminfo
PKG_VERSION:=2
CMAKE_INSTALL:=1

PKG_LICENSE:=BSD-2-Clause

PKG_FLAGS:=nonshared

include $(INCLUDE_DIR)/kernel.mk
include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/cmake.mk

define Package/libplatforminfo
  SECTION:=libs
  CATEGORY:=Libraries
  TITLE:=Platform information library
endef

define Package/libplatforminfo/config
	source "$(SOURCE)/Config.in"
endef

CMAKE_OPTIONS += \
	-DTARGET:STRING="$(BOARD)" \
	-DSUBTARGET:STRING="$(SUBTARGET)"

ifeq ($(CONFIG_LIBPLATFORMINFO_CUSTOM),y)
	CMAKE_OPTIONS += \
		-DCUSTOM:BOOL=ON \
		-DBOARD_NAME:STRING=$(CONFIG_LIBPLATFORMINFO_BOARD_NAME) \
		-DMODEL:STRING=$(CONFIG_LIBPLATFORMINFO_MODEL) \
		-DIMAGE_NAME:STRING=$(CONFIG_LIBPLATFORMINFO_IMAGE_NAME)
endif

define Package/libplatforminfo/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/lib/libplatforminfo.so $(1)/usr/lib/
endef

$(eval $(call BuildPackage,libplatforminfo))
