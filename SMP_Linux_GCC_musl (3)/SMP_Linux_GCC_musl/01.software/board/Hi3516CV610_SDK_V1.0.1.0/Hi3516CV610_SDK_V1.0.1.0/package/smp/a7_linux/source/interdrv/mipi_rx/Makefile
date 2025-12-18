ifeq ($(PARAM_FILE), )
    PARAM_FILE:=../../Makefile.param
    include $(PARAM_FILE)
endif

#*************************************************************************
# all source file in this module
SRCS := source/drv/osal/os/drv_mipirx_osal.c \
        source/drv/hal/hal_mipirx_comm.c \
        source/drv/hal/v100/hal_mipirx_crg.c \
        source/drv/hal/v100/hal_mipirx_mipi_ctrl.c \
        source/drv/hal/v100/hal_mipirx_lvds_ctrl.c \
        source/drv/hal/v100/hal_mipirx_sensor.c \
        source/drv/hal/v100/hal_mipirx_phy.c \
        source/drv/platform/linux/drv_mipirx_plt.c \
        source/drv/drv_mipirx_comm.c \
        source/drv/drv_mipirx_kapi.c \
        source/drv/drv_mipirx_ioctl.c \
        source/drv/drv_mipirx_proc.c \
#*************************************************************************
EXTRA_CFLAGS += -I$(PWD)/source/
EXTRA_CFLAGS += -I$(PWD)/source/drv \
                -I$(PWD)/source/drv/hal \
                -I$(PWD)/source/drv/hal/v100 \
                -I$(PWD)/source/drv/hal/v100/reg \
                -I$(PWD)/source/drv/osal/os \
                -I$(PWD)/source/drv/platform/linux \
                -I$(PWD)/source/include/kapi \
                -I$(PWD)/source/include/ioctl \
                -I$(PWD)
#-I$(PWD)/source/include/ioctl
EXTRA_CFLAGS += -I$(SYS_ROOT)/ext_inc/
EXTRA_CFLAGS += -I$(BASE_ROOT)/ext_inc/
ifeq ($(MPP_BUILD), y)
EXTRA_CFLAGS += -I$(DRV_ROOT)/$(OT_ARCH)/init/$(OSTYPE)
else
EXTRA_CFLAGS += -I$(DRV_ROOT)/init/$(OSTYPE)
endif
EXTRA_CFLAGS += -D$(OT_FPGA)

ifeq ($(OSTYPE),liteos)
EXTRA_CFLAGS += $(CFLAGS)
endif

TARGET := $(KO_PREFIX)_mipi_rx
INC_FILE := $(PWD)/source/include/ioctl/ot_mipi_rx.h
include $(MAKE_DRV_FILE)
