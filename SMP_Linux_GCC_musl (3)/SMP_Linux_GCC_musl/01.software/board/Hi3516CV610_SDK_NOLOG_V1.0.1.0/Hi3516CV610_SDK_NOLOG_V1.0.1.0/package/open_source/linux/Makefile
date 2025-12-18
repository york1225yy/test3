BUILD_DIR := $(shell pwd)

TOP_DIR:=$(BUILD_DIR)
MODULE_NAME:=linux

include ../scripts/Makefile.opensource

KERNEL_ORIGIN_VER:=linux-$(linux-version)
KERNEL_VER:=linux-5.10.y
KERNEL_TAR:=$(KERNEL_VER).tgz

OSDRV_DIR ?= ../../bsp
PUB_IMAGE ?=$(CHIP)$(MEDIUM_FLAG)image_$(LIB_TYPE)

ifeq ($(FASTBOOT_ENABLE),1)
ITS_FILE=linux_image_quickstart.its
IMAGE_NAME=Image
else
ITS_FILE=linux_image.its
IMAGE_NAME=zImage
endif

LINUX_KERNEL_ITS=$(OSDRV_DIR)/tools/pc/image_tree_source/$(ITS_FILE)

MAKE=make
CHIP ?= hi3519dv500

MKIMAGE_DIR := $(BUILD_DIR)/_mkimage

BOOT_MEDIA ?= emmc
ifeq ($(BOOT_MEDIA), spi)
MEDIUM_FLAG = _
DTB_SUFFIX=-flash
endif
ifeq ($(BOOT_MEDIA), nand)
MEDIUM_FLAG = _nand_
endif
ifeq ($(BOOT_MEDIA), emmc)
MEDIUM_FLAG = _emmc_
DTB_SUFFIX=-emmc
endif
ifeq ($(TEE_ENABLE),1)
	TEE_FLAG=tee_
endif

DTB_NAME=$(CHIP)-demb$(DTB_SUFFIX).dtb

CPU_TYPE ?=
ifeq ($(CHIP), hi3519dv500)
LINUX_SUFFIX := _$(CPU_TYPE)
endif

LIB_TYPE ?=musl
ARCH_TYPE ?= arm64
KERNEL_CFG ?=$(CHIP)$(MEDIUM_FLAG)$(TEE_FLAG)defconfig

ifeq ($(LIB_TYPE), musl)
OSDRV_CROSS ?= aarch64-v01c01-linux-musl
else ifeq ($(LIB_TYPE), glibc)
OSDRV_CROSS ?= aarch64-v01c01-linux-gnu
else
$(warning "---------------------------------------------------------------------")
$(warning "   LIB_TYPE Input Is ERR!!       ")
$(warning "---------------------------------------------------------------------")
$(error )
endif

CROSS := CROSS_COMPILE=$(OSDRV_CROSS)-
ifneq ($(LLVM), )
CROSS =
endif

GREEN = "\e[32;1m"
DONE    = "\033[0m"

all:
	@echo -e $(GREEN)"-------- kernel : $(KERNEL_ORIGIN_VER) "  $(DONE)

ifneq ($(BUILD_DIR)/$(KERNEL_VER), $(wildcard $(BUILD_DIR)/$(KERNEL_VER)))
	pushd $(BUILD_DIR);tar xf $(KERNEL_ORIGIN_VER).tar.gz;popd
	@echo -e $(GREEN)"patching kernel..."  $(DONE)
	pushd $(BUILD_DIR);mv $(KERNEL_ORIGIN_VER) $(KERNEL_VER);popd
	pushd $(BUILD_DIR)/$(KERNEL_VER); patch -p1< ../linux-$(linux-version).patch;popd
endif

	$(MAKE) -C $(BUILD_DIR)/$(KERNEL_VER) ARCH=$(ARCH_TYPE) LLVM=$(LLVM) LLVM_IAS=$(LLVM)  $(CROSS) distclean
	$(MAKE) -C $(BUILD_DIR)/$(KERNEL_VER) ARCH=$(ARCH_TYPE) LLVM=$(LLVM) LLVM_IAS=$(LLVM)  $(CROSS) $(KERNEL_CFG)
ifeq ($(CHIP_FAMILY),hi3516cv610)
ifeq ($(TEE_ENABLE),1)
	pushd $(BUILD_DIR)/$(KERNEL_VER);./scripts/config --file .config -e CONFIG_TEE -E CONFIG_TEE CONFIG_OPTEE --set-val CONFIG_OPTEE_SHM_NUM_PRIV_PAGES 1 -d CONFIG_SYNC_OPTEE_MEMORY_LAYOUT; popd
endif
ifeq ($(DEBUG),1)
	pushd $(BUILD_DIR)/$(KERNEL_VER)/arch/arm/boot/dts;sed -i '/&uart0 {/ {n;s/status = "disabled"/status = "okay"/}' $(CHIP)-demb.dts; popd
else
	pushd $(BUILD_DIR)/$(KERNEL_VER)/arch/arm/boot/dts;sed -i '/&uart0 {/ {n;s/status = "okay"/status = "disabled"/}' $(CHIP)-demb.dts; popd
endif
endif
ifeq ($(CHIP_FAMILY),hi3516cv610)
	rm $(MKIMAGE_DIR) -rf
	$(MAKE) -C $(BUILD_DIR)/$(KERNEL_VER) ARCH=$(ARCH_TYPE) LLVM=$(LLVM) LLVM_IAS=$(LLVM)  $(CROSS) zImage;
	$(MAKE) -C $(BUILD_DIR)/$(KERNEL_VER) ARCH=$(ARCH_TYPE) LLVM=$(LLVM) LLVM_IAS=$(LLVM)  $(CROSS) dtbs;
	mkdir $(MKIMAGE_DIR)
	cp $(BUILD_DIR)/$(KERNEL_VER)/arch/$(ARCH_TYPE)/boot/$(IMAGE_NAME) $(MKIMAGE_DIR)/
	cp $(BUILD_DIR)/$(KERNEL_VER)/arch/$(ARCH_TYPE)/boot/dts/$(DTB_NAME) $(MKIMAGE_DIR)/devicetree.dtb
	cp  $(LINUX_KERNEL_ITS) $(MKIMAGE_DIR)/
	pushd $(MKIMAGE_DIR);mkimage -f $(ITS_FILE) uImage; popd
	mv $(MKIMAGE_DIR)/uImage $(OSDRV_DIR)/pub/$(PUB_IMAGE)/
	rm $(MKIMAGE_DIR) -r
else
	$(MAKE) -C $(BUILD_DIR)/$(KERNEL_VER) ARCH=$(ARCH_TYPE) LLVM=$(LLVM) LLVM_IAS=$(LLVM)  $(CROSS) uImage;
endif

clean:
	rm $(BUILD_DIR)/$(KERNEL_ORIGIN_VER) -rf
	rm $(BUILD_DIR)/$(KERNEL_VER) -rf
	rm $(BUILD_DIR)/$(KERNEL_TAR) -rf
