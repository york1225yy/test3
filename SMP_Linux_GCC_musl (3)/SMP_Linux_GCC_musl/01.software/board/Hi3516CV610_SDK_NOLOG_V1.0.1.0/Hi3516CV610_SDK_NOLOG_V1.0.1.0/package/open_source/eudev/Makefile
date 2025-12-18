BUILD_DIR      := $(shell pwd)

TOP_DIR:=$(BUILD_DIR)
MODULE_NAME:=eudev

include ../scripts/Makefile.opensource

UDEV           := eudev-$(eudev-version)
ROOTFS         := rootfs
OSDRV_CROSS ?= aarch64-v01c01-linux-musl
HOST           := $(patsubst %-,%,$(OSDRV_CROSS))
ifneq ($(MINI_COMPILE),1)
OSDRV_CROSS_CFLAGS := -fstack-protector-strong -fPIE
OSDRV_CROSS_LDFLAGS := -Wl,-z,relro -Wl,-z,now -Wl,-z,noexecstack -pie
else
OSDRV_CROSS_CFLAGS := -fstack-protector-strong -fPIE -Os -mthumb -ffunction-sections -fdata-sections
OSDRV_CROSS_LDFLAGS := -Wl,-z,relro -Wl,-z,now -Wl,-z,noexecstack -pie -Wl,--gc-sections
endif

CLANG_CROSS :=
ifneq ($(LLVM),)
	CLANG_CROSS = CC=clang LD=ld.lld AS=clang AR=llvm-ar NM=llvm-nm STRIP=llvm-strip OBJCOPY=llvm-objcopy \
		                  OBJDUMP=llvm-objdump READELF=llvm-readelf RANLIB=llvm-ranlib
endif


all: prepare
	@rm $(BUILD_DIR)/$(ROOTFS) -rf
ifeq ($(CHIP_FAMILY),hi3519dv500)
	cd $(BUILD_DIR)/$(UDEV) && \
		./autogen.sh  && ./configure --prefix=/ --host=$(HOST) $(CLANG_CROSS) CFLAGS="$(OSDRV_CROSS_CFLAGS)" \
	 	LDFLAGS="$(OSDRV_CROSS_LDFLAGS)" --disable-introspection --disable-selinux 1>/dev/null;
	make -C $(BUILD_DIR)/$(UDEV) USE_STATIC=false OSDRV_CROSS=${OSDRV_CROSS} 1>/dev/null
	@mkdir -p $(BUILD_DIR)/$(ROOTFS)/etc/udev/rules.d/
	@cp $(BUILD_DIR)/$(UDEV)/rules/60-block.rules $(BUILD_DIR)/$(ROOTFS)/etc/udev/rules.d/
	@mkdir $(BUILD_DIR)/$(ROOTFS)/bin -p
	@cp $(BUILD_DIR)/$(UDEV)/src/udev/udevd $(BUILD_DIR)/$(ROOTFS)/bin
	@cp $(BUILD_DIR)/$(UDEV)/src/udev/udevadm $(BUILD_DIR)/$(ROOTFS)/bin
	@$(OSDRV_CROSS)-strip $(BUILD_DIR)/$(ROOTFS)/bin/*
else
	pushd $(BUILD_DIR)/$(UDEV); autoreconf -f -i -s; popd
ifneq ($(MINI_COMPILE),1)
	pushd $(BUILD_DIR)/$(UDEV) && \
		./configure --prefix=/ --host=$(HOST) $(CLANG_CROSS) CFLAGS="$(OSDRV_CROSS_CFLAGS)" \
			ac_cv_func_localtime_r=yes ac_cv_search_clock_gettime= \
			LDFLAGS="$(OSDRV_CROSS_LDFLAGS)" --disable-introspection --disable-selinux 1>/dev/null && popd
else
	pushd $(BUILD_DIR)/$(UDEV) && \
		./configure --prefix=/ --host=$(HOST) $(CLANG_CROSS) CFLAGS="$(OSDRV_CROSS_CFLAGS)" \
			ac_cv_func_localtime_r=yes ac_cv_search_clock_gettime= \
			LDFLAGS="$(OSDRV_CROSS_LDFLAGS)" --disable-introspection --disable-selinux --enable-minisize 1>/dev/null && popd
endif
	make -C $(BUILD_DIR)/$(UDEV) USE_STATIC=false OSDRV_CROSS=${OSDRV_CROSS} 1>/dev/null
	@mkdir $(OSDRV_DIR)/pub/$(PUB_ROOTFS)/etc/udev/rules.d/ -p
	@cp $(BUILD_DIR)/$(UDEV)/rules/60-block.rules $(OSDRV_DIR)/pub/$(PUB_ROOTFS)/etc/udev/rules.d/
	@mkdir $(OSDRV_DIR)/pub/$(PUB_ROOTFS)/bin -p
	@cp $(BUILD_DIR)/$(UDEV)/src/udev/udevd $(OSDRV_DIR)/pub/$(PUB_ROOTFS)/bin
	@cp $(BUILD_DIR)/$(UDEV)/src/udev/udevadm $(OSDRV_DIR)/pub/$(PUB_ROOTFS)/bin
endif

prepare:
ifeq ($(wildcard $(UDEV)),)
	@tar -xzf $(UDEV).tar.gz -C $(BUILD_DIR)
ifeq ($(CHIP_FAMILY),hi3516cv610)
ifeq ($(MINI_COMPILE),1)
ifeq ($(UDEV).patch, $(wildcard $(UDEV).patch))
	pushd $(BUILD_DIR)/$(UDEV);patch -p1 < ../$(UDEV).patch;popd;
endif
endif
endif
ifeq ($(CHIP_FAMILY),hi3519dv500)
ifeq ($(UDEV).patch, $(wildcard $(UDEV).patch))
	pushd $(BUILD_DIR)/$(UDEV);patch -p1 < ../$(UDEV).patch;popd;
endif
endif
endif

clean:
	@rm -rf $(BUILD_DIR)/$(ROOTFS)
	@rm -rf $(BUILD_DIR)/$(UDEV)

distclean: clean

.PHONY: clean distclean all prepare
