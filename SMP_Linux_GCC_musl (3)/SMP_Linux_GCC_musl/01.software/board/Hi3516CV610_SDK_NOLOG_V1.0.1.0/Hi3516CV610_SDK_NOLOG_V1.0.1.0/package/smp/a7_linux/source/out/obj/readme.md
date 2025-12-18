## Prepare
Please build kernel first, build drivers need KERNEL_ROOT,

## Build
### build drivers as modules
1. Use `make -j` to rebuild all drivers as modules, and build all sample/tools after that.
2. If kernel config is changed, all drivers must be rebuild.

### build drivers to kernel
1. Change Makefile.config to select which driver is need to build to kernel.
2. Use `make BUILD_DRIVER_TO_KERNEL=y -j` to build selected drivers to kernel.
3. go to bsp dir and use 'make CONFIG_MPPKO_BUILTIN=y kernel -j' to build uImage.

#### Note
1. If some drivers are build to the kernel, the corresponding module param need to be configured in
