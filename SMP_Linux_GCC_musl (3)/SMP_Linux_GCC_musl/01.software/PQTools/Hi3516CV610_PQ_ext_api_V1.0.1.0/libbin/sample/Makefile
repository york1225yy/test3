SDK_DIR := /home/xxx/Hi3516CV610_SDK_Vx.x.x.x
export OTARCH ?=Hi3516CV610

export CROSS := arm-v01c02-linux-musleabi-
#export CROSS := arm-v01c02-linux-gnueabi-
MPP_DIR = $(SDK_DIR)/smp/a7_linux/source/out

export SRC_DIR = $(shell pwd)
export INC_DIR = $(shell pwd)/../include
export BIN_LIB_DIR = $(shell pwd)/../release

TARGET := sample_bin

INC :=
INC += -I$(INC_DIR)
INC += -I$(MPP_DIR)/include

LIBS :=
LIBS+= $(BIN_LIB_DIR)/libbin.a
LIBS+= $(MPP_DIR)/lib/libss_mpi_isp.a
LIBS+= $(MPP_DIR)/lib/libot_mpi_isp.a
LIBS+= $(MPP_DIR)/lib/libcalcflicker.a
LIBS+= $(MPP_DIR)/lib/libss_mpi.a
LIBS+= $(MPP_DIR)/lib/libss_mpi_sysmem.a
LIBS+= $(MPP_DIR)/lib/libdehaze.a
LIBS+= $(MPP_DIR)/lib/libdrc.a
LIBS+= $(MPP_DIR)/lib/libldci.a
LIBS+= $(MPP_DIR)/lib/libsecurec.a
LIBS+= $(MPP_DIR)/lib/libir_auto.a
LIBS+= $(MPP_DIR)/lib/libacs.a
LIBS+= $(MPP_DIR)/lib/libbnr.a
LIBS+= $(MPP_DIR)/lib/libextend_stats.a
CC = $(CROSS)gcc

SRC := $(SRC_DIR)/sample_pq_bin.c
OBJ := $(SRC:%.c=%.o)

CFLAGS := -fPIC -Wall -lm -ldl -lpthread -lstdc++ -mcpu=cortex-a53

.PHONY: all $(TARGET) clean

all : $(TARGET)

$(TARGET) : $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^ $(LIBS)
$(OBJ) : $(SRC)
	$(CC) $(CFLAGS) -o $@ -c $< $(INC)

clean:
	@rm -f $(TARGET)
	@rm -f *.o
