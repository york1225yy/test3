SDK_DIR := /home/xxx/Hi3516CV610_SDK_Vx.x.x.x
CROSS := arm-v01c02-linux-musleabi-
#CROSS := arm-v01c02-linux-gnueabi-
SRC_DIR := $(shell pwd)
CONTROL_LIB_DIR := $(shell pwd)/../release

TARGET := test_control

LIBS :=
LIBS+= $(CONTROL_LIB_DIR)/lib_pqcontrol.a
LIBS+= -L$(SDK_DIR)/smp/a7_linux/source/out/lib

LIBS+= -lsecurec -lot_mpi_isp -lss_mpi_isp -lss_mpi -ldehaze -lbnr -lss_mpi_awb -lss_mpi_ae 
LIBS+= -lss_mpi_sysmem -lss_mpi_ive -lss_mpi_km -lsvp_acl -lcalcflicker
LIBS+= -lss_mpi_sysbind -lprotobuf-c -lupvqe -ldnvqe -lextend_stats -lir_auto -ldrc -lldci -lacs

CC = $(CROSS)gcc

SRC := $(SRC_DIR)/test_main.c
OBJ := $(SRC:%.c=%.o)

CFLAGS := -fPIC -Wall -lm -ldl -lpthread -lstdc++ -mcpu=cortex-a53

.PHONY: all $(TARGET) clean

all : $(TARGET)

$(TARGET) : $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^ $(LIBS)
$(OBJ) : $(SRC)
	$(CC) $(CFLAGS) -o $@ -c $<

clean:
	@rm -f $(TARGET)
	@rm -f *.o
