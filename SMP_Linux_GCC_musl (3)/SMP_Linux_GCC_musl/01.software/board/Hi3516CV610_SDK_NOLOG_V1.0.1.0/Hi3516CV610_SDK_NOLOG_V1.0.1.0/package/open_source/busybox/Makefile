TOOLS_TOP_DIR := $(shell pwd)

TOP_DIR:=$(TOOLS_TOP_DIR)
MODULE_NAME:=busybox
include ../scripts/Makefile.opensource

TOOL_NAME := busybox-$(busybox-version)
TOOL_TAR_BALL := $(TOOL_NAME).tar.bz2
TOOL_BIN := bin
ARCH ?= arm64

OSDRV_CROSS ?= aarch64-v01c01-linux-musl
OSDRV_DIR ?= ../../bsp
BUSYBOX_CFG ?= config_aarch64_a55_softfp_neon
CROSS = CROSS_COMPILE=$(OSDRV_CROSS)-
ifneq ($(LLVM),)
CROSS = CC=clang LD=ld.lld AS=clang AR=llvm-ar NM=llvm-nm STRIP=llvm-strip OBJCOPY=llvm-objcopy \
			              OBJDUMP=llvm-objdump READELF=llvm-readelf RANLIB=llvm-ranlib
BUSYBOX_CFG = config_aarch64_a55_softfp_neon_llvm
endif

ifeq ($(CHIP_FAMILY),hi3519dv500)
BUSYBOX_CFG_PATH:=$(TOOLS_TOP_DIR)/$(TOOL_NAME)/$(BUSYBOX_CFG)
else
BUSYBOX_CFG_PATH:=$(OSDRV_DIR)/tools/build_scripts/busybox/$(BUSYBOX_CFG)
endif

all:
	@echo "CROSS=$(CROSS)"
	@echo "BUSYBOX_CFG=$(BUSYBOX_CFG)"
ifeq ($(wildcard $(TOOLS_TOP_DIR)/$(TOOL_NAME)),)
	tar xjf $(TOOLS_TOP_DIR)/$(TOOL_TAR_BALL) -C $(TOOLS_TOP_DIR)
ifneq ($(wildcard $(TOOLS_TOP_DIR)/busybox-$(busybox-version).patch),)
	pushd $(TOOLS_TOP_DIR)/$(TOOL_NAME)/;patch -p1 < ../busybox-$(busybox-version).patch;popd
endif
endif
	cp $(BUSYBOX_CFG_PATH) $(TOOLS_TOP_DIR)/$(TOOL_NAME)/.config
	make ARCH=$(ARCH) $(CROSS) -C $(TOOLS_TOP_DIR)/$(TOOL_NAME)  1>/dev/null;
	make ARCH=$(ARCH) $(CROSS) -C $(TOOLS_TOP_DIR)/$(TOOL_NAME) install 1>/dev/null
	cp -af $(TOOLS_TOP_DIR)/$(TOOL_NAME)/_install/* $(OSDRV_DIR)/pub/$(PUB_ROOTFS)

clean:
	-rm $(TOOLS_TOP_DIR)/$(TOOL_NAME) -rf;

distclean: clean


