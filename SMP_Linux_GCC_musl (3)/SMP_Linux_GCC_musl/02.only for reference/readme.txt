特别说明：此说明仅用于使用hi接口操作，如使用ss接口，不涉及以下操作
Hi3516CV610_REF_VX.0.X.X.tgz作为补丁包，可以将此补丁包打到Hi3516CV610_SDK_VX.0.X.X.tgz包中，具体操作步骤如下：
1、将Hi3516CV610_SDK_VX.0.X.X.tgz和Hi3516CV610_REF_VX.0.X.X.tgz包放置到同一个文件夹下并解压，
    tar zxvf Hi3516CV610_SDK_VX.0.X.X.tgz
    tar zxvf Hi3516CV610_REF_VX.0.X.X.tgz
2、执行Hi3516CV610_SDK_VX.0.X.X文件夹内部解压，
    ./sdk.unpack
3、执行打补丁操作，
    ./ref_patch.sh

    执行后，Hi3516CV610_SDK_VX.0.X.X为打上REF补丁后的完整包。
