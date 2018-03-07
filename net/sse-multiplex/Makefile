include $(TOPDIR)/rules.mk

PKG_NAME:=sse-multiplex
PKG_VERSION:=2

PKG_LICENSE:=BSD-2-Clause

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/cmake.mk

define Package/sse-multiplex
  SECTION:=net
  CATEGORY:=Network
  DEPENDS:=
  TITLE:=Allows multiple clients to receive the same Server-Sent Event stream
endef

CMAKE_OPTIONS += -DSSE_MULTIPLEX_SOCKET:STRING=/var/run/sse-multiplex.sock

define Package/sse-multiplex/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/sbin/sse-multiplexd $(1)/usr/sbin/
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/sbin/sse-multiplex $(1)/usr/sbin/

	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) ./files/sse-multiplexd.init $(1)/etc/init.d/sse-multiplexd
endef

$(eval $(call BuildPackage,sse-multiplex))
