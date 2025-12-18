PREFIX := $(shell pwd)

TOP_DIR:=$(PREFIX)

MODULE_NAME:=zlib

include ../scripts/Makefile.opensource

ZLIB_NAME:=zlib-$(zlib-version)
ZLIB_TAR_BALL:=zlib-$(zlib-version).tar.gz

PC_DIR := $(shell pwd)/$(ZLIB_NAME)_pc
BOARD_DIR := $(shell pwd)/$(ZLIB_NAME)_board

OSDRV_CROSS ?= aarch64-v01c01-linux-musl
OSDRV_CROSS_CFLAGS := -fstack-protector-strong -fPIC
OSDRV_CROSS_LDFLAGS := -Wl,-z,relro -Wl,-z,now -Wl,-z,noexecstack

CROSS_CC :=
ifneq ($(LLVM),)
CROSS_CC = CC=clang LD=ld.lld AS=clang AR=llvm-ar NM=llvm-nm STRIP=llvm-strip OBJCOPY=llvm-objcopy \
	       OBJDUMP=llvm-objdump READELF=llvm-readelf RANLIB=llvm-ranlib
else
CROSS_CC = CC=$(OSDRV_CROSS)-gcc
endif

LOCAL_CFLAGS := -fPIC
LOCAL_LDFLAGS := -Wl,-z,relro -Wl,-z,now -Wl,-z,noexecstack
GCC_VERSION_GE49 := $(shell echo `gcc -dumpversion | cut -f1-2 -d.` \>= 4.9 | sed -e 's/\./*100+/g' | bc)
ifeq ($(GCC_VERSION_GE49),1)
LOCAL_CFLAGS += -fstack-protector-strong
else
LOCAL_CFLAGS += -fstack-protector-all
endif

all: zlib_pc  zlib_board

zlib_pc: $(PC_DIR)/lib/libz.a
$(PC_DIR)/lib/libz.a :
	rm $(PC_DIR) -rf
	rm -rf $(PREFIX)/out/pc
	mkdir $(PC_DIR)
	mkdir -p $(PREFIX)/out/pc
ifeq ($(wildcard $(PC_DIR)/$(ZLIB_NAME)),)
ifneq ($(wildcard $(ZLIB_NAME)),)
	cp -rf $(ZLIB_NAME) $(PC_DIR)/$(ZLIB_NAME)
else
	tar -xf $(ZLIB_TAR_BALL) -C $(PC_DIR)
endif
ifneq ($(wildcard $(ZLIB_NAME).patch), )
	pushd $(PC_DIR)/$(ZLIB_NAME); patch -p1 < ../../$(ZLIB_NAME).patch; popd
endif
endif
	cd $(PC_DIR)/$(ZLIB_NAME)/ && \
		CFLAGS="$(LOCAL_CFLAGS)" LDFLAGS="$(LOCAL_LDFLAGS)" ./configure --prefix="$(PC_DIR)" && \
		make CC=gcc clean>/dev/null && \
		make CC=gcc 1>/dev/null && \
		make install CC=gcc 1>/dev/null

	cp $(PC_DIR)/include $(PREFIX)/out/pc -r
	cp $(PC_DIR)/lib $(PREFIX)/out/pc -r
	cp $(PC_DIR)/share $(PREFIX)/out/pc -r
	rm $(PC_DIR) -rf

zlib_board: $(BOARD_DIR)/lib/libz.a
$(BOARD_DIR)/lib/libz.a :
	rm $(BOARD_DIR) -rf
	rm -rf $(PREFIX)/out/board
	mkdir $(BOARD_DIR)
	mkdir -p $(PREFIX)/out/board
ifeq ($(wildcard $(BOARD_DIR)/$(ZLIB_NAME)), )
ifneq ($(wildcard $(ZLIB_NAME)),)
	cp -rf $(ZLIB_NAME) $(BOARD_DIR)/$(ZLIB_NAME)
else
	tar -xf $(ZLIB_TAR_BALL) -C $(BOARD_DIR)
endif
ifneq ($(wildcard $(ZLIB_NAME).patch), )
	pushd $(BOARD_DIR)/$(ZLIB_NAME); patch -p1 < ../../$(ZLIB_NAME).patch; popd
endif
endif
	cd $(BOARD_DIR)/$(ZLIB_NAME)/ && \
		prefix=zlib_install $(CROSS_CC) CFLAGS="$(OSDRV_CROSS_CFLAGS)" \
		LDFLAGS="$(OSDRV_CROSS_LDFLAGS)" ./configure && \
		make 1>/dev/null && \
		make install 1>/dev/null;

	cp $(BOARD_DIR)/$(ZLIB_NAME)/zlib_install/* $(PREFIX)/out/board -r
	rm $(BOARD_DIR) -rf

clean:
	rm out -rf;
	rm $(ZLIB_NAME)_* -rf
distclean: clean
