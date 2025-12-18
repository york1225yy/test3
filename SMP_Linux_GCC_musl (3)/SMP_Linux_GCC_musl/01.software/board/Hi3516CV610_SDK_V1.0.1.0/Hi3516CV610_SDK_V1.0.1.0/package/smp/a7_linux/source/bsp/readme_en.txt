1.How to use Makefile of directory bsp:
	1) Since there are many dependency components for compiling a ROOTFS in Makefile, it is not guaranteed that a separately compiled ROOTFS is available. "make all" compiler is recommended.
	2) uboot, kernel can be compiled separately.
	3) This readme describes hi3516cv610.
	4) The compilation scripts in this directory support the use of arm-v01c02-linux-musleabi and arm-v01c02-linux-gnueabi 32bit toolchains for compilation;
	5) The arm-v01c02-linux-musleabi toolchain corresponds to the musl library;
	6) During overall compilation, if you need to use the musl library toolchain, please configure LIB_TYPE=musl;
	7) During separating compilation. If you need to use the musl library toolchain, please configure CROSS_COMPILE=arm-v01c02-linux-musleabi-;
	8) Taking the musl cross toolchain compilation command as an example.

Note:
	Due to the reform of open source tools, some open source tools no longer provide source code packages. Customers need to download them by themselves before compiling OSDRV:
	a:linux-5.10.xx.tar.gz  (https://www.kernel.org/pub/). Place the downloaded linux-5.10.xx.tar.gz in the open_source/linux directory.
	  The version of the Linux kernel compressed package needs to be consistent with the version of the patch file.


(1)Compile the entire bsp directory:
	make all
	Default Compilation:
	make BOOT_MEDIA=spi LIB_TYPE=musl DEBUG=0 all

	Compilation parameter description：
        1)BOOT_MEDIA selects the spi startup method for compilation, and can choose the emmc startup method for compilation, which is BOOT_MEDIA=emmc;If the NAND flash of the release version is used, BOOT_MEDIA=spi_nand must be transferred.
        2)LIB_TYPE defaults to musl compilation.
        3)CHIP defaults to hi3516cv610, which can be optionally changed to Hi3516CV610 or Hi3516CV608;
        4)DEBUG defaults to DEBUG=0 to compile the release version of the system image. DEBUG=1 to compile the debug version of the system image.
	5)OS image file will be builded to directory of bsp/pub

(2)Clear all compiled files under bsp directory:
	make clean

(3)Completely remove all compiled files under bsp directory, and the generated images:
	make distclean

(4)Separately compile kernel:
	make kernel -j

	To modify the config configuration and recompile the kernel image, perform the following steps:
	1) Go to the open_source/linux/linux-5.10.y directory, run the make ARCH=arm CROSS_COMPILE=arm-v01c02-linux-musleabi-menuconfig command, and modify the config file.
	2) Run the make ARCH=arm CROSS_COMPILE=arm-v01c02-linux-musleabi- savedefconfig command to generate the defconfig file.
	3) Copy the generated defconfig file as a new deconfig file and run the cp defconfig arch/arm/configs/hi3516cv610_new_defconfig command.
	4) Go to the bsp directory and run "make KERNEL_CFG=hi3516cv610_new_defconfig kernel" again (Note: add argument "BOOT_MEDIA=emmc" if boot media is eMMC).

(5)Separately compile non-sercure boot image:
	make gslboot_build  -j

	If you are not using the default boot table, you need to copy the table to tools/pc/boot_tools and add the parameter REGBIN_XLSM to specify the boot table when compiling the boot. For example, run the following command in bsp/ directory：
	make gslboot_build REGBIN_XLSM=xxxx.xlsm -j

(6)Build file system image(take hi3516cv610 for example):
    After compiling the entire bsp directory, the compiled file system and default file system images will be generated in the bsp/pub/directory. If the specifications of the startup media used by the user do not match the default file system images, the file system images can be made according to the specifications of the startup media on the single board.

	Filesystem images of jffs2 format is available for spi flash. While making jffs2 image or squashfs image, you need to specify the spi flash block size.


	The flash block size will be printed when uboot runs. run mkfs.jffs2 first to get the right parameters from it's printed information.
	Here the block size is 64KB, for example:
	./bsp/pub/bin/pc/mkfs.jffs2 -d bsp/pub/rootfs_musl_arm -l -e 0x10000 -o bsp/pub/rootfs_hi3516cv610_64k.jffs2

	UBIFS format image is available for nand flash. Pagesize and blocksize of nand flash are needed when making UBIFS image.
	This information will be printed when uboot starts.The following example is 2KB page size, 128KB block size:
	./bsp/tools/pc/ubi_sh/mkubiimg.sh hi3516cv610 2k 128k bsp/pub/rootfs_musl_arm 32M bsp/pub/bin/pc

	Emmc uses ext4 image format, taking 96MB image as an example:(need prepare rootfs_hi3516cv610_96M.ext4、rootfs_musl_arm、populate-extfs.sh)
	dd if=/dev/zero of=bsp/pub/hi3516cv610_emmc_image_musl/rootfs_hi3516cv610_96M.ext4 bs=512 count=196608
	note:(196608 = 96 * 1024 * 1024 / 512)
	./bsp/pub/bin/pc/mkfs.ext4 bsp/pub/hi3516cv610_emmc_image_musl/rootfs_hi3516cv610_96M.ext4
	enter dir: open_source/e2fsprogs/out/pc/contrib:
	./populate-extfs.sh ../../../../../smp/a7_linux/source/bsp/pub/rootfs_musl_arm bsp/pub/hi3516cv610_emmc_image_musl/rootfs_hi3516cv610_96M.ext4

2.Note
(1)Executable files in Linux may become non-executable after copying them to somewhere else under Windows, and result in souce code cannot be compiled. Many symbolic link files will be generated in the souce package after compiling the u-boot or the kernel. The volume of the source package will become very big, because all the symbolic link files are turning out to be real files in Windows. So, DO NOT copy source package in Windows.
(2) This chip has a floating-point unit and neon. The libraries in the file system are compiled with hard floating-point and neon using a compatible soft floating-point calling interface. Therefore, users should note that all code on this chip board needs to add the following items in the Makefile during compilation: -mcpu=cortex-a7, -mfloat-abi=softfp, and -mfpu=neon-vfpv4.
For example, for A7:
	CFLAGS += -mcpu=cortex-a7 -mfloat-abi=softfp -mfpu=neon-vfpv4
	CXXFlAGS += -mcpu=cortex-a7 -mfloat-abi=softfp -mfpu=neon-vfpv4
