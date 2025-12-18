ifeq ($(PARAM_FILE), )
     PARAM_FILE := ../../Makefile.param
     include $(PARAM_FILE)
endif

ifeq ($(CONFIG_OT_GDC_FISHEYE_SUPPORT), y)
     LIBS += $(REL_LIB)/libss_mpi_fisheye_calibrate.a
     SRC += fisheye_calibration.c
endif

ifeq ($(CONFIG_OT_AVS_SUPPORT), y)
     SRC += avs_chn_dump.c avs_src_dump.c
endif

ifeq ($(CONFIG_OT_VI_ALL_SUPPORT), y)
     SRC += vi_pipe_dump.c vi_chn_dump.c
else ifeq ($(CONFIG_OT_VI_XVR_SUPPORT), y)
     SRC += vi_dump.c
endif

ifeq ($(CONFIG_OT_AUDIO_SUPPORT), y)
    SRC += ai_dump.c ao_dump.c
	LIBS += $(REL_LIB)/libss_mpi_audio.a
	LIBS += $(REL_LIB)/libvoice_engine.a
	LIBS += $(REL_LIB)/libupvqe.a
	LIBS += $(REL_LIB)/libdnvqe.a
endif

ifeq ($(CONFIG_OT_MCF_SUPPORT), y)
	SRC += mcf_chn_dump.c mcf_grp_dump.c
	LIBS += $(REL_LIB)/libss_mpi_mcf.a
endif

ifeq ($(CONFIG_OT_AIISP_SUPPORT), y)
	SRC += aiisp_dump.c
	CFLAGS += -I$(SDK_PATH)/source/mpp/cbb/vb/include/inner
ifeq ($(CONFIG_OT_AIBNR_SUPPORT), y)
	CFLAGS += -DOT_AIBNR_SUPPORT
	LIBS += $(REL_LIB)/libss_mpi_aibnr.a
endif
ifeq ($(CONFIG_OT_AI3DNR_SUPPORT), y)
	CFLAGS += -DOT_AI3DNR_SUPPORT
	LIBS += $(REL_LIB)/libss_mpi_ai3dnr.a
endif
ifeq ($(CONFIG_OT_SVP_SUPPORT), y)
	LIBS += $(REL_LIB)/libsvp_acl.a
	LIBS += $(REL_LIB)/libss_mpi_km.a
	LIBS += $(REL_LIB)/libprotobuf-c.a
endif
endif

ifeq ($(CONFIG_OT_DIS_SUPPORT), y)
	SRC += dis_dump.c dis_send.c
	LIBS += $(REL_LIB)/libss_mpi_motionfusion.a
endif

ifeq ($(CONFIG_OT_VPSS_SUPPORT), y)
	SRC += vpss_src_dump.c vpss_chn_dump.c
ifeq ($(CONFIG_OT_XVR_SUPPORT), y)
	SRC += vpss_attr.c
endif
endif

ifeq ($(CONFIG_OT_VO_SUPPORT), y)
	SRC += vo_chn_dump.c vo_config.c vo_screen_dump.c
endif

ifeq ($(CONFIG_MIPI_TX), y)
	SRC += mipitx_read.c mipitx_write.c
endif

ifeq ($(CONFIG_OT_SVP_DPU_MATCH), y)
	SRC += dpu_match_dump.c
	LIBS += $(REL_LIB)/libss_mpi_dpu_match.a
endif

OBJ := $(SRC:%.c=%.o)

INC_FLAGS := -I$(REL_INC)
INC_FLAGS += -I$(REL_INC)
CFLAGS += -Wall -g $(INC_FLAGS)
CFLAGS += $(LIBS_CFLAGS)
CFLAGS += $(LIBS_EXEC_CFLAGS)

LIBS += $(REL_LIB)/libss_mpi.a
LIBS += $(REL_LIB)/libss_mpi_sysbind.a
LIBS += $(REL_LIB)/libss_mpi_sysmem.a
LIBS += $(REL_LIB)/libsecurec.a

ifeq ($(CONFIG_OT_ISP_SUPPORT), y)
LIBS += $(REL_LIB)/libss_mpi_ae.a
LIBS += $(REL_LIB)/libss_mpi_isp.a
LIBS += $(REL_LIB)/libot_mpi_isp.a
LIBS += $(REL_LIB)/libss_mpi_awb.a
LIBS += $(REL_LIB)/libdehaze.a
LIBS += $(REL_LIB)/libextend_stats.a
LIBS += $(REL_LIB)/libdrc.a
LIBS += $(REL_LIB)/libldci.a
LIBS += $(REL_LIB)/libbnr.a
LIBS += $(REL_LIB)/libcalcflicker.a
LIBS += $(REL_LIB)/libir_auto.a
LIBS += $(REL_LIB)/libacs.a
ifeq ($(CONFIG_OT_ISP_DUMP_DEBUG_SUPPORT), y)
LIBS += $(REL_LIB)/libdump_dbg.a
endif
endif

LIBS += $(LIBS_LD_CFLAGS)
CFLAGS += $(LIBS_CFLAGS)
ifneq ($(CONFIG_LLVM), y)
CFLAGS += -lpthread -ldl -lstdc++ -lm
endif

include $(PWD)/Make.$(OSTYPE)
