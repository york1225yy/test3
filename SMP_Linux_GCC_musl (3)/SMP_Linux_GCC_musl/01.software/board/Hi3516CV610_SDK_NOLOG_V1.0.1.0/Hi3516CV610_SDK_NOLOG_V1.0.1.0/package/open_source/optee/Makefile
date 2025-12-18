####################################################
#
# make all
# make clean
#
# make optee_os
# make optee_os_clean
#
# make optee_client
# make optee_client_clean
#
####################################################
BUILD_DIR := $(shell pwd)

include $(BUILD_DIR)/Makefile.param
OPTEE_OS_ZIP ?= $(OPTEE_OS_VER).zip
OPTEE_CLIENT_ZIP ?= $(OPTEE_CLIENT_VER).zip
OPTEE_SAMPLE := example
MUSL_LIB_DIR:=$(BUILD_DIR)/../musl
MUSL_TAR_FILE:=$(MUSL_LIB_DIR)/musl-1.2.3.tar.gz

unexport ARCH
MAKE=make
OSDRV_CROSS ?= aarch64-v01c01-linux-musl
CROSS_COMPILE=$(OSDRV_CROSS)-

CHIP ?= hi3519dv500
PLAT ?= vendor

GREEN = "\e[32;1m"
DONE  = "\033[0m"

CFLAGS += -fPIE -fstack-protector-all
LDFLAGS += -s -z relro -z now

LIB_TYPE ?= musl
OPTEE_OS_BUILD_ARG :=
OPTEE_CLIENT_BUILD_ARG :=

ifeq ($(LIB_TYPE), musl)
	CFG_GP_SOCKETS=n
endif

ifeq ($(LLVM),1)
	COMPILER=clang
endif

ifeq ($(CHIP),hi3519dv500)
OPTEE_EXPORT_NAME = export-ta_arm64
MUSL_LIB = musl_lib
MUSL_LIB_CLEAN = musl_lib_clean
CHIP_FAMILY = hi3519dv500
else ifeq ($(CHIP),hi3516dv500)
OPTEE_EXPORT_NAME = export-ta_arm64
MUSL_LIB = musl_lib
MUSL_LIB_CLEAN = musl_lib_clean
CHIP_FAMILY = hi3516dv500
else
OPTEE_EXPORT_NAME = export-ta_arm32
MUSL_LIB =
MUSL_LIB_CLEAN =
CHIP_FAMILY = hi3516cv610
endif

###############################################  optee_os     ###########################################
optee_os: $(MUSL_LIB)
	@echo -e $(GREEN)"-------- optee_os : $(OPTEE_OS_VER) "  $(DONE)
ifneq ($(BUILD_DIR)/$(OPTEE_OS_VER), $(wildcard $(BUILD_DIR)/$(OPTEE_OS_VER)))
	pushd $(BUILD_DIR); unzip $(OPTEE_OS_ZIP);popd
	@echo -e $(GREEN)"patching optee_os..."  $(DONE)
	pushd $(BUILD_DIR)/$(OPTEE_OS_VER); patch -p1 < ../$(OPTEE_OS_VER).patch;popd
	@echo -e $(GREEN)"patched optee_os!"  $(DONE)
endif
	find $(BUILD_DIR)/$(OPTEE_OS_VER) | xargs touch
	@if [ ! -d $(BUILD_DIR)/$(OPTEE_OS_VER)/keys ]; then \
        	echo "Creating folder $(BUILD_DIR)/$(OPTEE_OS_VER)/keys"; \
        	mkdir -p $(BUILD_DIR)/$(OPTEE_OS_VER)/keys; \
	fi
	@if [ ! -f $(BUILD_DIR)/$(OPTEE_OS_VER)/keys/default_ta.pem ]; then \
		echo "default_ta.pem does not exist, creating..."; \
		openssl genrsa -out $(BUILD_DIR)/$(OPTEE_OS_VER)/keys/default_ta.pem 4096 || (echo "openssl command failed, please install openssl." && exit 1); \
	else \
		echo "$(BUILD_DIR)/$(OPTEE_OS_VER)/keys/default_ta.pem already exists."; \
	fi
	cd $(BUILD_DIR)/$(OPTEE_OS_VER) && \
		$(MAKE) CROSS_COMPILE=$(CROSS_COMPILE) PLATFORM=$(PLAT)-$(CHIP_FAMILY)_demo -j 1>/dev/null 2>&1;
ifneq ($(OPTEE_PATH), )
	@rm $(OPTEE_PATH) -rf;
	cp -r $(BUILD_DIR)/$(OPTEE_OS_VER)/out/arm-plat-vendor/$(OPTEE_EXPORT_NAME) $(OPTEE_PATH);
	@rm $(OPTEE_PATH)/ta -rf;
	@rm $(OPTEE_PATH)/keys/default_ta.pem -rf;
	@rm $(OPTEE_PATH)/host_include -rf;
endif

optee_os_clean: $(MUSL_LIB_CLEAN)
ifneq ($(OPTEE_PATH), )
	@rm $(OPTEE_PATH) -rf;
endif
	@rm $(BUILD_DIR)/$(OPTEE_OS_VER)/out -rf;

musl_lib:
	@echo -e $(GREEN)"---------task [10.0] build musl_lib"  $(DONE)
	$(MAKE) -C $(MUSL_LIB_DIR)

musl_lib_clean:
	$(MAKE) -C $(MUSL_LIB_DIR) clean
###############################################  optee_client  ###########################################
optee_client:
	@echo -e $(GREEN)"-------- optee_client : $(OPTEE_CLIENT_VER) "  $(DONE)
ifneq ($(BUILD_DIR)/$(OPTEE_CLIENT_VER), $(wildcard $(BUILD_DIR)/$(OPTEE_CLIENT_VER)))
	pushd $(BUILD_DIR); unzip $(OPTEE_CLIENT_ZIP);popd
ifneq ($(wildcard $(OPTEE_CLIENT_VER).patch), )
	pushd $(BUILD_DIR)/$(OPTEE_CLIENT_VER); patch -p1 < ../$(OPTEE_CLIENT_VER).patch; popd
endif
endif
	find $(BUILD_DIR)/$(OPTEE_CLIENT_VER) | xargs touch
	cd $(BUILD_DIR)/$(OPTEE_CLIENT_VER) && \
		$(MAKE) CFLAGS="$(CFLAGS)" LDFLAGS="$(LDFLAGS)" COMPILER=$(COMPILER) CROSS_COMPILE=$(CROSS_COMPILE) WITH_TEEACL=0 \
				CFG_GP_SOCKETS=$(CFG_GP_SOCKETS) PLATFORM=$(PLAT)-$(CHIP_FAMILY)_demo -j;
ifneq ($(OPTEE_PATH), )
	@cp $(BUILD_DIR)/$(OPTEE_CLIENT_VER)/out/export/usr/lib/libteec.a $(REL_LIB);
	@cp $(BUILD_DIR)/$(OPTEE_CLIENT_VER)/out/export/usr/lib/libteec.so.1 $(REL_LIB);
	@cp $(BUILD_DIR)/$(OPTEE_CLIENT_VER)/out/export/usr/include/tee_client_api.h $(REL_INC);
endif

optee_client_clean:
ifneq ($(OPTEE_PATH), )
	@rm $(REL_LIB)/libteec* -rf;
	@rm $(REL_INC)/tee_client_api.h -rf;
endif
	@rm $(BUILD_DIR)/$(OPTEE_CLIENT_VER)/out -rf;

.PHONY: all optee_os optee_client clean optee_os_clean optee_client_clean

###############################################       all      ###########################################
all: optee_os optee_client
clean: optee_os_clean optee_client_clean
