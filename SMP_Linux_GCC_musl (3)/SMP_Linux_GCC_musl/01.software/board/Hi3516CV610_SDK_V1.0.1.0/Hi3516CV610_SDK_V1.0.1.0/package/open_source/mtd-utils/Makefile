TOOL_DIR := $(shell pwd)

TOP_DIR:=$(TOOL_DIR)
MODULE_NAME:=mtd-utils

include ../scripts/Makefile.opensource

TOOL_BIN := bin
OSDRV_CROSS ?= aarch64-v01c01-linux-musl
TOOL_NAME:=mtd-utils-$(mtd-utils-version)
TOOL_TAR_BALL := $(TOOL_NAME).tar.bz2

BOARD_ZLIB_LIB_PATH :=$(TOOL_DIR)/../zlib/out/board/lib/libz.a

PC_DIR := $(shell pwd)/$(TOOL_NAME)_pc
BOARD_DIR := $(shell pwd)/$(TOOL_NAME)_board
ZLIB_PC_DIR := $(TOOL_DIR)/../zlib/out/pc

OSDRV_CROSS_CFLAGS := -fstack-protector-strong -fPIC
OSDRV_CROSS_LDFLAGS := -Wl,-z,relro -Wl,-z,now -Wl,-z,noexecstack -pie

INCLUDE += -I$(TOOL_DIR)/../zlib/out/board/include

LOCAL_CFLAGS := -fPIC
LOCAL_LDFLAGS := -Wl,-z,relro -Wl,-z,now -Wl,-z,noexecstack
GCC_VERSION_GE49 := $(shell echo `gcc -dumpversion | cut -f1-2 -d.` \>= 4.9 | sed -e 's/\./*100+/g' | bc)
ifeq ($(GCC_VERSION_GE49),1)
LOCAL_CFLAGS += -fstack-protector-strong
else
LOCAL_CFLAGS += -fstack-protector-all
endif

CLANG_CROSS :=
ifneq ($(LLVM),)
CLANG_CROSS = CC=clang LD=ld.lld AS=clang AR=llvm-ar NM=llvm-nm STRIP=llvm-strip OBJCOPY=llvm-objcopy \
		      OBJDUMP=llvm-objdump READELF=llvm-readelf RANLIB=llvm-ranlib
endif
all:  mtd_pc mtd_board

mtd_board:
ifneq ($(BOARD_ZLIB_LIB_PATH), $(wildcard $(BOARD_ZLIB_LIB_PATH)))
	$(warning "---------------------------------------------------------------------")
	$(warning "     Cannot found the $(BOARD_ZLIB_LIB_PATH) file                    ")
	$(warning "     Please to make zlib package                                ")
	$(warning "---------------------------------------------------------------------")
	$(error )
endif
	rm $(BOARD_DIR) -rf;
	rm -rf $(TOOL_DIR)/out/board
	mkdir $(BOARD_DIR)
	mkdir -p $(TOOL_DIR)/out/board

ifneq ($(wildcard $(TOOL_NAME)),)
	cp -rf $(TOOL_NAME) $(BOARD_DIR)
else
	tar -xf $(TOOL_TAR_BALL) -C $(BOARD_DIR)
endif
ifneq ($(wildcard $(TOOL_NAME).patch), )
	pushd $(BOARD_DIR)/$(TOOL_NAME); patch -p1 < ../../$(TOOL_NAME).patch; popd
endif
	mkdir -p $(BOARD_DIR)/$(TOOL_NAME)/$(TOOL_BIN)/;
	mkdir -p $(BOARD_DIR)/$(TOOL_BIN)/;
	@echo $(BOARD_DIR)/$(TOOL_NAME)/configure
	test -f $(BOARD_DIR)/$(TOOL_NAME)/configure || ( pushd $(BOARD_DIR)/$(TOOL_NAME)/; ./autogen.sh; popd )
	cd $(BOARD_DIR)/$(TOOL_NAME)/ && \
		./configure --without-ubifs --without-xattr --without-lzo --without-zstd --without-crypto \
		--host=$(OSDRV_CROSS) $(CLANG_CROSS) CFLAGS="$(OSDRV_CROSS_CFLAGS) $(INCLUDE)" LDFLAGS="$(OSDRV_CROSS_LDFLAGS)" \
		--prefix=$(BOARD_DIR)/$(TOOL_NAME)/$(TOOL_BIN) \
		ZLIB_LIBS=$(BOARD_ZLIB_LIB_PATH) --disable-tests && \
		make 1>/dev/null && \
		make install 1>/dev/null;

	cp -rf $(BOARD_DIR)/$(TOOL_NAME)/$(TOOL_BIN)/sbin/* $(BOARD_DIR)/$(TOOL_BIN)/
	cp $(BOARD_DIR)/$(TOOL_BIN)/  $(TOOL_DIR)/out/board -r
	rm $(BOARD_DIR) -rf
	file $(TOOL_DIR)/out/board/bin/*|grep ELF|cut -d ':' -f 1|xargs $(OSDRV_CROSS)-strip

mtd_pc:
	rm $(PC_DIR) -rf
	rm -rf $(TOOL_DIR)/out/pc
	mkdir $(PC_DIR)
	mkdir -p $(TOOL_DIR)/out/pc
ifneq ($(wildcard $(TOOL_NAME)),)
	cp -rf $(TOOL_NAME) $(PC_DIR)
else
	tar -xf $(TOOL_TAR_BALL) -C $(PC_DIR)
endif
ifneq ($(wildcard $(TOOL_NAME).patch), )
	pushd $(PC_DIR)/$(TOOL_NAME); patch -p1 < ../../$(TOOL_NAME).patch; popd
endif
	test -f $(PC_DIR)/$(TOOL_NAME)/configure || ( pushd $(PC_DIR)/$(TOOL_NAME)/; ./autogen.sh; popd )
	cd $(PC_DIR)/$(TOOL_NAME) && \
		./configure --prefix=$(PC_DIR) CC=gcc \
		--without-xattr --without-crypto --without-lzo --without-zstd \
		    CFLAGS="$(LOCAL_CFLAGS) -I$(ZLIB_PC_DIR)/include"  \
			LDFLAGS="$(LOCAL_LDFLAGS) -L$(ZLIB_PC_DIR)/lib" \
			ZLIBCPPFLAGS="-I$(ZLIB_PC_DIR)/include" \
			ZLIBLDFLAGS="-L$(ZLIB_PC_DIR)/lib" && \
		make 1>/dev/null  && \
		make install 1>/dev/null;

	cp $(PC_DIR)/sbin/mkfs.ubifs $(TOOL_DIR)/out/pc/
	cp $(PC_DIR)/sbin/ubinize $(TOOL_DIR)/out/pc/
	cp $(PC_DIR)/sbin/mkfs.jffs2 $(TOOL_DIR)/out/pc/
	rm $(PC_DIR) -rf
	file $(TOOL_DIR)/out/pc/*|grep ELF|cut -d ':' -f 1|xargs strip
clean:
	rm $(PC_DIR) -rf;
	rm $(BOARD_DIR) -rf;
	rm $(TOOL_DIR)/out -rf

distclean: clean
