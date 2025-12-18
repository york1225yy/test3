#!/bin/bash

mipi_csi_rx_part_path=$(realpath $(realpath $0 | xargs dirname)/../../../../)
linux_path=$(realpath ../../../../open_source/linux/linux-5.10.y)
product=$(realpath $0 | xargs dirname | xargs basename)
build_script_path=$mipi_csi_rx_part_path/parts/mipi_csi_rx/script/$product

BUILD_DRV=0
BUILD_TEST=0
function print_error()
{
    local red_head="\033[31m"
    local red_end="\033[0m"
    echo -e ${red_head}"[ERROR] $1"${red_end}
    exit 1
}

function print_help()
{
    local script_file=$(basename $0)
    cat << EOF
    Usage: ./$script_file [-h] [drv] [test]


    for example:
        ./$script_file drv
EOF
}

function parse_arguments()
{
    if [ $# -lt 1 ]; then
        print_help
        exit 1
    fi

    local input_arg=""
    while [ "$1" != "" ]; do
        input_arg=$1
        echo $input_arg
        case $input_arg in
            -h|--help)
                print_help
                exit 0
                ;;
            drv)
                BUILD_DRV=1
                ;;
            test)
                BUILD_TEST=1
                ;;
            *)
                print_error "Invalid input: $input_arg"
                exit 1
        esac
        shift
    done
}

if [ ! -d "$mipi_csi_rx_part_path" ]; then
    print_error "$mipi_csi_rx_part_path No such directory"
fi

function build()
{
    parse_arguments ${@}

    ln -snf $mipi_csi_rx_part_path/parts/mipi_csi_rx/source source
    ln -snf $mipi_csi_rx_part_path/parts/mipi_csi_rx/test test

    if [ $BUILD_DRV -eq 1 ]; then
        echo "build drv"
        ln -snf $build_script_path/Makefile_drv Makefile
        make KERNEL_ROOT=$linux_path -j
        if [ $? -ne 0 ]; then
            print_error "Build drv failed."
            exit 1
        fi
    fi

    if [ $BUILD_TEST -eq 1 ]; then
        echo "build test"
        # build kunit ko
        ln -snf $build_script_path/Makefile_kunit Makefile
        make KERNEL_ROOT=$linux_path -j
        if [ $? -ne 0 ]; then
            print_error "Build kunit failed."
            exit 1
        fi
        # build kunit test ko
        ln -snf $build_script_path/Makefile_kunit_test Makefile
        make KERNEL_ROOT=$linux_path -j
        if [ $? -ne 0 ]; then
            print_error "Build kunit test failed."
            exit 1
        fi
        # build cunit test cunit
        ln -snf $build_script_path/Makefile_cunit_test Makefile
        make KERNEL_ROOT=$linux_path -j
        if [ $? -ne 0 ]; then
            print_error "Build cunit test failed."
            exit 1
        fi
    fi
}

build ${@}
