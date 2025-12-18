BUILD_DIR := $(shell pwd)

TOP_DIR:=$(BUILD_DIR)

MODULE_NAME:=u-boot
include ../scripts/Makefile.opensource

OSDRV_DIR ?= ../../bsp

MAKE=make
CHIP ?= hi3519dv500
CHIP_FAMILY ?= hi3519dv500

UBOOT_VER ?=u-boot-$(u-boot-version)
UBOOT_TAR:=$(UBOOT_VER).tar.bz2
LIB_TYPE ?=glibc
ARCH_TYPE ?= arm64
ARM_ARCH_VERSION ?=armv8
UBOOT_REG_BIN :=$(CHIP)_reg_info.bin
UBOOT_PATCH_NAME:=u-boot-$(u-boot-version)

BOOT_MEDIA ?= spi
ifeq ($(BOOT_MEDIA), spi)
MEDIUM_FLAG = _
ENV_NAME = nor$(TEE_FLAG)_env
endif
ifeq ($(BOOT_MEDIA), nand)
MEDIUM_FLAG = _nand_
endif
ifeq ($(DEBUG),0)
ifeq ($(BOOT_MEDIA), spi_nand)
MEDIUM_FLAG = _
ENV_NAME = nand$(TEE_FLAG)_env
endif
endif
ifeq ($(BOOT_MEDIA), emmc)
MEDIUM_FLAG = _emmc_
ENV_NAME = emmc$(TEE_FLAG)_env
endif

GZIP_TOOL ?= gzip
GZIP_DIR ?= ../gzip
GZIP_VER ?= gzip-$(gzip-version)
OSDRV_CROSS ?= aarch64-v01c01-linux-musl
UBOOT_CONFIG ?=$(CHIP)$(MEDIUM_FLAG)defconfig
PUB_IMAGE ?=$(CHIP)$(MEDIUM_FLAG)image_$(LIB_TYPE)
BUILD ?= part

GREEN = "\e[32;1m"
DONE    = "\033[0m"

ifneq ($(LLVM), )
CROSS =
else
CROSS = CROSS_COMPILE=$(OSDRV_CROSS)-
endif

all:
	@echo -e $(GREEN)"-------- uboot : $(UBOOT_VER) "  $(DONE)

ifneq ($(BUILD_DIR)/$(UBOOT_VER), $(wildcard $(BUILD_DIR)/$(UBOOT_VER)))
	pushd $(BUILD_DIR);tar xjf $(UBOOT_TAR) -C .;popd
	pushd $(BUILD_DIR)/$(UBOOT_VER);patch -p1 < ../$(UBOOT_PATCH_NAME).patch;popd
endif
	find $(BUILD_DIR)/$(UBOOT_VER) | xargs touch
	cd $(BUILD_DIR)/$(UBOOT_VER) && \
		$(MAKE) $(CROSS) distclean >/dev/null;

	$(MAKE) -C $(BUILD_DIR)/$(UBOOT_VER) LLVM=$(LLVM) $(CROSS) $(UBOOT_CONFIG)
ifeq ($(CHIP_FAMILY),hi3516cv610)
ifeq ($(DEBUG),0)
	cp $(OSDRV_DIR)/tools/pc/uboot_env/env_text/$(CHIP)/$(ENV_NAME).txt $(BUILD_DIR)/$(UBOOT_VER)/board/vendor/hi3516cv610/hi3516cv610.env
	pushd $(BUILD_DIR)/$(UBOOT_VER)/board/vendor/hi3516cv610;sed -i 's/bootdelay=2/bootdelay=-2/' hi3516cv610.env;popd
endif
endif
	pushd $(BUILD_DIR)/$(UBOOT_VER);$(MAKE) LLVM=$(LLVM) $(CROSS) 1>/dev/null;popd
ifeq ($(GZIP_TOOL),gzip)
	echo "gzip"
ifeq ($(wildcard $(BUILD_DIR)/../gzip/bin/gzip), )
	make -C $(BUILD_DIR)/../gzip/
endif
	cp $(BUILD_DIR)/../gzip/bin/gzip $(BUILD_DIR)/$(UBOOT_VER)/arch/arm/cpu/$(ARM_ARCH_VERSION)/$(CHIP_FAMILY)/hw_compressed -rf
	chmod +x $(BUILD_DIR)/$(UBOOT_VER)/arch/arm/cpu/$(ARM_ARCH_VERSION)/$(CHIP_FAMILY)/hw_compressed/gzip
endif
	echo "gzip end"
	pushd $(BUILD_DIR)/$(UBOOT_VER);$(MAKE) LLVM=$(LLVM) $(CROSS) u-boot-z.bin 1>/dev/null;popd

	pushd $(BUILD_DIR)/$(UBOOT_VER);$(MAKE) LLVM=$(LLVM) $(CROSS) u-boot-z.clean 1>/dev/null;popd

ifneq ($(BOOT_IMAGE_TOOL_INPUT_DIR),)
	cp $(UBOOT_VER)/u-boot-$(CHIP).bin $(BOOT_IMAGE_TOOL_INPUT_DIR)/u-boot-original.bin;
endif

clean:
	rm $(BUILD_DIR)/$(UBOOT_VER) -rf
	$(MAKE) -C $(GZIP_DIR) clean

