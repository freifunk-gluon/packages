include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/target.mk

PKG_NAME:=libplatforminfo
PKG_VERSION:=2
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
