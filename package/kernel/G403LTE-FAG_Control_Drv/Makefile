#
# Copyright (C) 2008-2012 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=G403LTE-FAG_Control_Drv
PKG_RELEASE:=1

include $(INCLUDE_DIR)/package.mk

define KernelPackage/G403LTE-FAG_Control_Drv
  SUBMENU:=Other modules
  # DEPENDS:=@!LINUX_3_3
  TITLE:=Motor driver
  FILES:=$(PKG_BUILD_DIR)/G403LTE-FAG_Control_Drv.ko
  AUTOLOAD:=$(call AutoLoad,30,G403LTE-FAG_Control_Drv,1)
  KCONFIG:=
endef

define KernelPackage/G403LTE-FAG_Control_Drv/description
 This is a G403LTE-FAG_Control_Drv drivers
 endef

MAKE_OPTS:= \
	ARCH="$(LINUX_KARCH)" \
	CROSS_COMPILE="$(TARGET_CROSS)" \
	SUBDIRS="$(PKG_BUILD_DIR)"

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./src/* $(PKG_BUILD_DIR)/
endef

define Build/Compile
	$(MAKE) -C "$(LINUX_DIR)" \
		$(MAKE_OPTS) \
		modules
endef

$(eval $(call KernelPackage,G403LTE-FAG_Control_Drv))
