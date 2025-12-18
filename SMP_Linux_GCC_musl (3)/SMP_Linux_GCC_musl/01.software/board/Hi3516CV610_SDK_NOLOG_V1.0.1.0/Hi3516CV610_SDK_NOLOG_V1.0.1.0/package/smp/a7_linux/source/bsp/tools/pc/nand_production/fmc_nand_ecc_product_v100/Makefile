#===============================================================================

# local variable
#===============================================================================

OSDRV_DIR ?= $(shell pwd)/../../../..
SECURECDIR ?= $(OSDRV_DIR)/components/secure_c
SECUREC_INCDIR ?= $(SECURECDIR)/include/
SECUREC_LIBDIR ?= $(SECURECDIR)/libs/
SECURE_LIB = $(wildcard $(SECUREC_LIBDIR)/libsecurec-pc.a)

APP = nand_product
#OBJS = nandc_ecc.o oob_config_v100.o main.o nandc_random.o memset_s.o memcpy_s.o
OBJS = nandc_ecc.o oob_config_v100.o main.o nandc_random.o

LOCAL_CFLAGS := -fPIE
LOCAL_LDFLAGS := -Wl,-z,relro -Wl,-z,now -Wl,-z,noexecstack -fPIE -pie
GCC_VERSION_GE49 := $(shell echo `gcc -dumpversion | cut -f1-2 -d.` \>= 4.9 | sed -e 's/\./*100+/g' | bc)
ifeq ($(GCC_VERSION_GE49),1)
LOCAL_CFLAGS += -fstack-protector-strong
else
LOCAL_CFLAGS += -fstack-protector-all
endif
#===============================================================================
# rules
#===============================================================================
.PHONY: all clean install uninstall

all:$(APP)

%.o: %.c
	$(AT)$(CC) $(LOCAL_CFLAGS) -I$(SECUREC_INCDIR) -c -o $@ $^

$(APP): ${OBJS}

ifneq ($(SECURE_LIB), $(SECUREC_LIBDIR)/libsecurec-pc.a)
	$(error you need to build libsecurec-pc.a first!!!)
endif

	$(AT)$(CC) $(LOCAL_LDFLAGS) $^ -o $@ -lsecurec-pc -lpthread -L$(SECUREC_LIBDIR)
	$(AT)strip $(APP)

clean:
	-$(AT)rm -f ${OBJS} ${APP}
	-$(AT)rm -f libsecurec-pc.a

install:

uninstall:
