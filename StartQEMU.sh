#!/bin/bash

input_arg_1=$1 
input_arg_2=$2

binaries=$(find $directory -type f -name "*.bin")
target_binary=""
target_binary_mission=""
target_binary_devel=""

set -- ${binaries}
if [ $# == 0 ];then
echo No valid binary found!
fi

echo $# Binaries found: 
for binary in ${binaries}; do
    echo ${binary}
    if [[ ${binary} == *"devel"* ]];then
        target_binary_devel=${binary}
    elif [[ ${binary} == *"mission"* ]];then
        target_binary_mission=${binary}
    fi
done

if [ "${target_binary_mission}" != "" ];then
    if [ ${input_arg_2} == "mission" ];then
        target_binary=$target_binary_mission
        echo Mission binary $target_binary_mission selected
    else
        echo Devel binary $target_binary_devel selected
        target_binary=$target_binary_devel
        echo 
    fi
else
    echo Devel binary selected
    target_binary=$target_binary_devel
fi

if [ ${target_binary} == "" ]; then
    echo No valid binary found
    if [ $# == 1 ];then
	echo Setting the one binary ${target_binary} found
    	target_binary=binaries
    fi
    exit
else 
    echo Binary selected: 
    echo ${target_binary}
fi
echo

echo Launching QEMU:
echo

if [ "$input_arg_1" == "no-wait" ];then
../obc-qemu/build/arm-softmmu/qemu-system-arm -M \
isis-obc -serial stdio -monitor none \
-bios ${target_binary} -s 
else
../obc-qemu/build/arm-softmmu/qemu-system-arm -M \
isis-obc -serial stdio -monitor none \
-bios ${target_binary} -S -s 
fi

# Run this if you want to use telnet monitoring
# Run telnet localhost 55555 in separate console to connect.

#../obc-qemu/build/arm-softmmu/qemu-system-arm -M \
#isis-obc -monitor telnet:localhost:55555,server -serial stdio \
#-bios _bin/sam9g20/devel/sourceobsw-at91sam9g20_ek-sdram.bin 


