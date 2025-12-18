#!/bin/bash
version=Hi3516CV610_SDK_NOLOG_V1.0.1.0

sdk_package=${version}
ref_package=${version/SDK_/REF_}
#================initial==========================
base_path=$(cd `dirname $0`; pwd)

sdk_path=smp/a7_linux/source
rm ${sdk_package}/${sdk_path}/out/lib/* -rf
rm ${sdk_package}/${sdk_path}/out/include/* -rf
cp ${ref_package}/${sdk_path}/out/lib/* ${sdk_package}/${sdk_path}/out/lib/ -rf
cp ${ref_package}/${sdk_path}/out/include/* ${sdk_package}/${sdk_path}/out/include/ -rf

mv ${sdk_package}/${sdk_path}/mpp/sample/svp/svp_npu/data/model ${ref_package}/${sdk_path}/mpp/sample/svp/svp_npu/data
rm ${sdk_package}/${sdk_path}/mpp/sample -rf
cp ${ref_package}/${sdk_path}/mpp/sample ${sdk_package}/${sdk_path}/mpp -rf

rm ${sdk_package}/${sdk_path}/mpp/tools -rf
cp ${ref_package}/${sdk_path}/mpp/tools ${sdk_package}/${sdk_path}/mpp -rf

rm ${sdk_package}/${sdk_path}/mpp/cbb/audio -rf
cp ${ref_package}/${sdk_path}/mpp/cbb/audio ${sdk_package}/${sdk_path}/mpp/cbb -rf

rm ${sdk_package}/${sdk_path}/scripts -rf
cp ${ref_package}/${sdk_path}/scripts ${sdk_package}/${sdk_path} -rf
