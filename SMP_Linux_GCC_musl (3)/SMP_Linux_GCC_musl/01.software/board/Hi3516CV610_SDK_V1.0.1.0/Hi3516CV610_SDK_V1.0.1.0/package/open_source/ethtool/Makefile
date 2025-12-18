TOOLS_TOP_DIR ?= $(shell pwd)

TOP_DIR:=$(TOOLS_TOP_DIR)
MODULE_NAME:=ethtool

include ../scripts/Makefile.opensource

TOOL_TAR_BALL := ethtool-$(ethtool-version).tar.xz
TOOL_NAME := ethtool-$(ethtool-version)
TOOL_BIN := bin
OSDRV_CROSS ?= aarch64-v01c01-linux-musl
OSDRV_CROSS_CFLAGS := -fstack-protector-strong -fPIE
OSDRV_CROSS_LDFLAGS := -Wl,-z,relro -Wl,-z,now -Wl,-z,noexecstack -pie
CLANG_CROSS :=
ifneq ($(LLVM),)
	CLANG_CROSS = CC=clang LD=ld.lld AS=clang AR=llvm-ar NM=llvm-nm STRIP=llvm-strip OBJCOPY=llvm-objcopy \
		      OBJDUMP=llvm-objdump READELF=llvm-readelf RANLIB=llvm-ranlib
endif

all:
ifeq ($(wildcard $(TOOL_NAME)),)
	tar xf $(TOOL_TAR_BALL);
ifneq ($(wildcard $(TOOL_NAME).patch), )
	pushd $(TOOLS_TOP_DIR)/$(TOOL_NAME); patch -p1 < ../$(TOOL_NAME).patch; popd
endif
endif
	mkdir -p $(TOOLS_TOP_DIR)/$(TOOL_BIN)/;
	mkdir -p $(TOOLS_TOP_DIR)/$(TOOL_NAME)/$(TOOL_BIN)/;

	cd $(TOOLS_TOP_DIR)/$(TOOL_NAME)/ && \
		./configure --host=$(OSDRV_CROSS) $(CLANG_CROSS) CFLAGS="$(OSDRV_CROSS_CFLAGS)" \
		LDFLAGS="$(OSDRV_CROSS_LDFLAGS)" --enable-netlink=no \
		--prefix=$(TOOLS_TOP_DIR)/$(TOOL_NAME)/$(TOOL_BIN) > /dev/null && \
		make -j 20 > /dev/null && \
		make install > /dev/null && \
		cp $(TOOL_BIN)/* $(TOOLS_TOP_DIR)/$(TOOL_BIN) -rf;
		$(OSDRV_CROSS)-strip $(TOOLS_TOP_DIR)/$(TOOL_BIN)/sbin/*;

.PHONY: clean
clean:
	@rm $(TOOLS_TOP_DIR)/$(TOOL_NAME) -rf;
	@rm $(TOOLS_TOP_DIR)/$(TOOL_BIN) -rf;

.PHONY: distclean
distclean:
	-rm $(TOOLS_TOP_DIR)/$(TOOL_NAME) -rf;
	-rm $(TOOLS_TOP_DIR)/$(TOOL_BIN) -rf;

