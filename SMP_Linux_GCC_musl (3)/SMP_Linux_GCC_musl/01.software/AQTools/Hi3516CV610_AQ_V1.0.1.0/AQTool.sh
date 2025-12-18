#!/bin/sh
# Copyright (c) CompanyNameMagicTag 2023-2023. All rights reserved.
# Description:
# Author: pqtool team
# Create: 2023/6/1

fun_kill() {
program=$1
killall $program
timeout=0
while true;do
    programId=$(pidof $program)
    if [ -z "$programId" ];then
        echo    ">>>>no pid,run it"
        break
    else
        echo "PID:" $programId
        echo ">>>>program is exiting waiting"
        usleep 200000 # 200 ms
        timeout=$(expr $timeout + 1)
    fi
    if [ $timeout -eq 5 ];then
        if [ -z "$programId" ];then
            :
        else
            wait $programId
            kill -9 $programId
        fi
        break
    fi
done
}

DLL_PATH=${LD_LIBRARY_PATH}:${PWD}/libs
fun_kill ittb_aqtool
export LD_LIBRARY_PATH=${DLL_PATH}
ulimit -s 1024
./ittb_aqtool $1 & 
 
