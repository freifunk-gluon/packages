include $(TOPDIR)/rules.mk

PKG_NAME:=uc
PKG_VERSION:=1

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL=https://github.com/christf/uc.git
PKG_SOURCE_SUBDIR:=$(PKG_NAME)-$(PKG_VERSION)
PKG_SOURCE_VERSION:=d143ee1536f0a12d6cf5fadeb7a2df158495dffe
PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION)-$(PKG_SOURCE_VERSION).tar.gz

PKG_INSTALL:=1

include $(INCLUDE_DIR)/package.mk

define Package/uc
  SECTION:=utils
  CATEGORY:=Utilities
  TITLE:=small utility to write to a unix socket and obtain response
  DEPENDS:=
endef

define Package/uc/description
  uc can be used to pipe a command into a unix socket and to read and display the corresponding response.
endef

define Package/uc/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/uc $(1)/usr/bin
endef

$(eval $(call BuildPackage,uc))
