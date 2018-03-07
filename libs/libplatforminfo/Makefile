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

CMAKE_OPTIONS += \
	-DTARGET:STRING="$(BOARD)" \
	-DSUBTARGET:STRING="$(SUBTARGET)"


define Package/libplatforminfo/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/lib/libplatforminfo.so $(1)/usr/lib/
endef

$(eval $(call BuildPackage,libplatforminfo))
