####################################################
#
# make all
# make clean
#
# mbedtls_primary
# primary_clean
# make mbedtls_harden_build
# make mbedtls_harden_clean
#
####################################################
BUILD_DIR := $(shell pwd)

PARAM_FILE_SDK_A55:=../../smp/a55_linux/source/Makefile.param
PARAM_FILE_SDK_A7:=../../smp/a7_linux/source/Makefile.param
PARAM_FILE_SRCS:=../../source/Makefile.param
ifneq ($(wildcard $(PARAM_FILE_SDK_A55)),)
    PARAM_FILE:=$(PARAM_FILE_SDK_A55)
    include $(PARAM_FILE)
else ifneq ($(wildcard $(PARAM_FILE_SDK_A7)),)
    PARAM_FILE:=$(PARAM_FILE_SDK_A7)
    include $(PARAM_FILE)
else
    PARAM_FILE:=$(PARAM_FILE_SRCS)
    include $(PARAM_FILE)
endif

ifdef CONFIG_HI3519DV500
	MBEDTLS_VER := mbedtls-3.1.0
endif
ifdef CONFIG_HI3516CV610
	MBEDTLS_VER := mbedtls-3.6.0
endif

MBEDTLS_ZIP := $(MBEDTLS_VER).zip

export CFLAGS += $(LIBS_CFLAGS)
export CFLAGS += $(LIBS_LD_CFLAGS)

###############################################  mbedtls_primary     ###########################################
primary_build:
	@echo -e $(GREEN)"--------primary mbedtls : $(MBEDTLS_VER) "  $(DONE)
ifneq ($(BUILD_DIR)/$(MBEDTLS_VER), $(wildcard $(BUILD_DIR)/$(MBEDTLS_VER)))
	pushd $(BUILD_DIR); unzip $(MBEDTLS_ZIP);popd
	@echo -e $(GREEN)"patching mbedtls..."  $(DONE)
	pushd $(BUILD_DIR)/$(MBEDTLS_VER); patch -p1 < ../$(MBEDTLS_VER).patch;popd
	@echo -e $(GREEN)"patched mbedtls!"  $(DONE)
endif
	cd $(MBEDTLS_VER) && touch tests/suites/* && $(MAKE) CC=$(CC) lib -j;

###############################################  mbedtls_harden     ###########################################
mbedtls_harden_build:
	@echo -e $(GREEN)"--------harden mbedtls : $(MBEDTLS_VER) "  $(DONE)
ifneq ($(BUILD_DIR)/$(MBEDTLS_VER), $(wildcard $(BUILD_DIR)/$(MBEDTLS_VER)))
	pushd $(BUILD_DIR); unzip $(MBEDTLS_ZIP);popd
	@echo -e $(GREEN)"patching mbedtls..."  $(DONE)
	pushd $(BUILD_DIR)/$(MBEDTLS_VER); patch -p1 < ../$(MBEDTLS_VER).patch;popd
	@echo -e $(GREEN)"patched mbedtls!"  $(DONE)
endif
	find $(BUILD_DIR)/$(MBEDTLS_VER)/harden/build/vision | xargs touch
	cd $(BUILD_DIR)/$(MBEDTLS_VER)/harden/build/vision && \
		$(MAKE) -j;

mbedtls_harden_clean:
ifneq ($(wildcard $(BUILD_DIR)/$(MBEDTLS_VER)),)
	cd $(BUILD_DIR)/$(MBEDTLS_VER)/harden/build/vision && \
		$(MAKE) clean;
endif

primary_clean:
ifneq ($(wildcard $(BUILD_DIR)/$(MBEDTLS_VER)),)
	cd $(BUILD_DIR)/$(MBEDTLS_VER) && $(MAKE) -C library clean;
endif

.PHONY: all primary_build mbedtls_harden_build clean primary_clean

###############################################       all      ###########################################
all: primary_build
harden: mbedtls_harden_build
clean: mbedtls_harden_clean primary_clean
