include $(TOPDIR)/rules.mk

PKG_NAME:=simple-tc
PKG_VERSION:=1

PKG_CONFIG_DEPENDS := CONFIG_KERNEL_NET_SCH_TBF CONFIG_KERNEL_NET_SCH_INGRESS CONFIG_KERNEL_NET_CLS_BASIC KERNEL_NET_ACT_POLICE

include $(INCLUDE_DIR)/package.mk

define Package/simple-tc
  SECTION:=net
  CATEGORY:=Network
  TITLE:=Simple bandwidth limiting
  DEPENDS:=+!KERNEL_NET_SCH_TBF:kmod-sched-core +!KERNEL_NET_SCH_INGRESS:kmod-sched-core +!KERNEL_NET_CLS_BASIC:kmod-sched +!KERNEL_NET_ACT_POLICE:kmod-sched +libnl-tiny
endef

TARGET_CFLAGS += -I$(STAGING_DIR)/usr/include/libnl-tiny

define Package/simple-tc/install
	$(CP) ./files/* $(1)/
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/simple-tc $(1)/usr/sbin/
endef

$(eval $(call BuildPackage,simple-tc))
