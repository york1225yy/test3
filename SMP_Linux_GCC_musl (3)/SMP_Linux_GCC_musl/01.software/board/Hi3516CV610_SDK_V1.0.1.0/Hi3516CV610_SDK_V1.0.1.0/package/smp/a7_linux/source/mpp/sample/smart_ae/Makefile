include ../Makefile.param

##################################define AIDETECT_SUPPORT at here to support aidetect##################################################
#CFLAGS += -D AIDETECT_SUPPORT
#MPI_LIBS += -L$(REL_LIB) -lss_mpi_aidetect
#LIBS_LD_CFLAGS += -L$(REL_LIB) -lsecurec -lss_mpi -lss_mpi_sysmem -lsvp_acl -lprotobuf-c -lss_mpi_km -lss_mpi_aidetect -lss_mpi_cipher
#######################################################################################################################################

MPI_LIBS += $(REL_LIB)/libss_mpi_smartae.a
MPI_LIBS += $(REL_LIB)/libss_mpi_cipher.a

########### SRCS & INC ############
SMP_SRCS := $(wildcard $(PWD)/*.c)

TARGET := sample_smart_ae

TARGET_PATH := $(PWD)

# compile linux or LITEOS
include $(PWD)/../$(ARM_ARCH)_$(OSTYPE).mak
