#
# Copyright (C) 2008-2012 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=gl-hw-wdt
PKG_RELEASE:=1
PKG_LICENSE:=GPL-2.0

include $(INCLUDE_DIR)/package.mk

define KernelPackage/gl-hw-wdt
  SUBMENU:=Other modules
  TITLE:=gl hardware watchdog driver
  FILES:=$(PKG_BUILD_DIR)/gl_hw_wdt.ko
  AUTOLOAD:=$(call AutoLoad,30,gl_hw_wdt,1)
  KCONFIG:=
endef

define KernelPackage/gpio-button-hotplug/description
 This is a replacement for the following in-kernel drivers:
 1) gpio_keys (KEYBOARD_GPIO)
 2) gpio_keys_polled (KEYBOARD_GPIO_POLLED)

 Instead of generating input events (like in-kernel drivers do) it generates
 uevent-s and broadcasts them. This allows disabling input subsystem which is
 an overkill for OpenWrt simple needs.
endef

EXTRA_KCONFIG:= \
	CONFIG_BUTTON_HOTPLUG=m

EXTRA_CFLAGS:= \
	$(patsubst CONFIG_%, -DCONFIG_%=1, $(patsubst %=m,%,$(filter %=m,$(EXTRA_KCONFIG)))) \
	$(patsubst CONFIG_%, -DCONFIG_%=1, $(patsubst %=y,%,$(filter %=y,$(EXTRA_KCONFIG)))) \

#EXTRA_CFLAGS+=-DCC_HAVE_ASM_GOTO -DUSE_IMMEDIATE

MAKE_OPTS:= \
	ARCH="$(LINUX_KARCH)" \
	CROSS_COMPILE="$(TARGET_CROSS)" \
	SUBDIRS="$(PKG_BUILD_DIR)" \
	EXTRA_CFLAGS="$(EXTRA_CFLAGS)" \
	$(EXTRA_KCONFIG)

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./src/* $(PKG_BUILD_DIR)/
endef

define Build/Compile
	$(MAKE) -C "$(LINUX_DIR)" \
		$(MAKE_OPTS) \
		modules
endef

$(eval $(call KernelPackage,gl-hw-wdt))
