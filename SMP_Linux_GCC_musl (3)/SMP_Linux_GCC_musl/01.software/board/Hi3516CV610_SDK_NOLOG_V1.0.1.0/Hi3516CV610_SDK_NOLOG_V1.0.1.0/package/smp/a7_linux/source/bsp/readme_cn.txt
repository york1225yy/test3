1.bsp 顶层 Makefile 使用说明
	1)由于Makefile中文件系统编译依赖组件较多，不能保证单独编译的文件系统可用，建议采用make all编译；
	2)可单独编译uboot，kernel;
	3)本readme针对hi3516cv610进行说明;
	4)本目录下的编译脚本支持选用arm-v01c02-linux-musleabi、arm-v01c02-linux-gnueabi 32bit工具链进行编译;
	5)其中arm-v01c02-linux-musleabi工具链对应musl库;
	6)总体编译时，如需使用musl库工具链, 请配置LIB_TYPE=musl;
	7)单独编译时，如需使用musl库工具链, 请配置CROSS_COMPILE=arm-v01c02-linux-musleabi-;
	8)以下以musl交叉工具链编译命令为例.


注意：
	由于开源工具整改，部分开源工具不再提供源码包，客户编译OSDRV时需要自行下载：
	a:linux-5.10.xx.tar.gz (https://www.kernel.org/pub/), 将下载的 linux-5.10.xx.tar.gz 存放到 open_source/linux 目录中
	  其中linux-5.10.xx需要与patch的版本匹配.

(1)编译整个bsp目录：
	make all
	默认编译:
	make LIB_TYPE=musl CHIP=hi3516cv610 DEBUG=0 all

	编译参数说明：
	1)BOOT_MEDIA默认选择spi启动方式编译，可选择emmc启动方式编译,即BOOT_MEDIA=emmc；若使用release版本的nand介质，需要传入BOOT_MEDIA=spi_nand。
	2)LIB_TYPE默认为musl编译。
	3)CHIP默认为hi3516cv610，可选择hi3516cv610或hi3516cv608编译,即CHIP=hi3516cv610。
	4)DEBUG默认为DEBUG=0，表示编译 release 版本系统镜像。可选择为DEBUG=1，即编译debug版本系统镜像。
	5)bsp/pub目录下文件即为所编译小系统镜像

(2)清除整个bsp目录的编译文件：
	make  clean

(3)彻底清除整个bsp目录的编译中间文件：
	make distclean

(4)单独编译kernel image：
	make kernel

	修改config配置，重编kernel镜像的方法：
	1)进入open_source/linux/linux-5.10.y目录，执行make ARCH=arm CROSS_COMPILE=arm-v01c02-linux-musleabi- menuconfig，修改config配置
	2)执行make ARCH=arm CROSS_COMPILE=arm-v01c02-linux-musleabi- savedefconfig，生成defconfig文件
	3)将生成的defconfig文件拷贝为新的deconfig文件，执行cp defconfig arch/arm/configs/hi3516cv610_new_defconfig
	4)进入bsp目录，重新执行make KERNEL_CFG=hi3516cv610_new_defconfig kernel (注：eMMC 介质需加上 BOOT_MEDIA=emmc 参数)

(5)单独编译非安全启动镜像：
	make gslboot_build  -j

	如果没有使用默认表格, 需要把表格拷贝到 tools/pc/boot_tools, 在编译 boot 时加参数 REGBIN_XLSM 指定表格, 如在 bsp/ 目录下执行:
	make gslboot_build REGBIN_XLSM=xxxx.xlsm -j


(6)制作文件系统镜像(以hi3516cv610为例)：
在bsp/pub/中有已经编译好的文件系统，因此无需再重复编译文件系统，只需要根据单板上启动介质的规格型号制作文件系统镜像即可。

	spi flash使用jffs2格式的镜像，制作jffs2镜像时，需要用到spi flash的块大小。这些信息会在uboot启动时会打印出来。建议使用时先直接运行mkfs.jffs2工具，根据打印信息填写相关参数。下面以块大小为64KB为例：
	./bsp/pub/bin/pc/mkfs.jffs2 -d bsp/pub/rootfs_musl_arm -l -e 0x10000 -o bsp/pub/rootfs_hi3516cv610_64k.jffs2

	nand flash使用ubifs格式的镜像，制作ubifs镜像时，需要用到nand flash的pagesize和blocksize。这些信息会在uboot启动时会打印出来。
	下面以2KB pagesize、128KB block size为例：
	./bsp/tools/pc/ubi_sh/mkubiimg.sh hi3516cv610 2k 128k bsp/pub/rootfs_musl_arm 32M bsp/pub/bin/pc

	emmc 使用ext4格式的镜像：以96MB镜像为例:(需要准备rootfs_hi3516cv610_96M.ext4、rootfs_musl、populate-extfs.sh)
	dd if=/dev/zero of=bsp/pub/hi3516cv610_emmc_image_musl/rootfs_hi3516cv610_96M.ext4 bs=512 count=196608
	备注:(196608 = 96 *1024 * 1024 / 512)
	./bsp/pub/bin/pc/mkfs.ext4 bsp/pub/hi3516cv610_emmc_image_musl/rootfs_hi3516cv610_96M.ext4
	进入 open_source/e2fsprogs/out/pc/contrib:
	./populate-extfs.sh ../../../../../smp/a7_linux/source/bsp/pub/rootfs_musl_arm ../../../../../smp/a7_linux/source/bsp/pub/hi3516cv610_emmc_image_musl/rootfs_hi3516cv610_96M.ext4

2.注意事项
(1)在windows下复制源码包时，linux下的可执行文件可能变为非可执行文件，导致无法编译使用；u-boot或内核下编译后，会有很多符号链接文件，

在windows下复制这些源码包, 会使源码包变的巨大，因为linux下的符号链接文件变为windows下实实在在的文件，导致源码包膨胀。
因此使用时请注意不要在windows下复制源代码包。
2) 此芯片具有浮点运算单元和neon。文件系统中的库是采用兼容软浮点调用接口的硬浮点和neon编译而成，因此请用户注意，所有此芯片板端代码编译时需要在Makefile里面添加选项-mcpu=cortex-a7、-mfloat-abi=softfp和-mfpu=neon-vfpv4。
如：
对于A7：
    CFLAGS += -mcpu=cortex-a7 -mfloat-abi=softfp -mfpu=neon-vfpv4
    CXXFlAGS += -mcpu=cortex-a7 -mfloat-abi=softfp -mfpu=neon-vfpv4
