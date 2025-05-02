include $(TOPDIR)/rules.mk

PKG_NAME:=wifi-mesh-macfilter
PKG_VERSION:=1
PKG_RELEASE:=1

PKG_BUILD_DIR := $(BUILD_DIR)/$(PKG_NAME)

include $(INCLUDE_DIR)/package.mk

define Package/wifi-mesh-macfilter
  SECTION:=net
  CATEGORY:=NETWORK
  TITLE:=WiFi - block/open 802.11s-mesh stations
  DEPENDS:=+iw +uci
endef

define Package/wifi-mesh-macfilter/description
	Setting plink_action [block|open], when 802.11s mesh-interface comes up.
	Define filterpolicy in uci wireless.<wifi-iface>.macfilter=[disable|deny|allow].
	Deny sets iw dev <device> station set <mac> plink_action block, and
	allow sets iw dev <device> mesh_param mesh_auto_open_plink=0 and iw dev <device> station set <mac> plink_action open.
	List MAC adresses (divided by spaces) in uci wireless.<wifi-iface>.maclist.
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
endef

define Build/Configure
endef

define Build/Compile
endef

define Package/wifi-mesh-macfilter/install
	$(CP) ./files/* $(1)/
endef

$(eval $(call BuildPackage,wifi-mesh-macfilter))
