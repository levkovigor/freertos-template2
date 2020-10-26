#!/bin/bash
req_directory="../obc-qemu"
if [ ! -d "$req_directory" ]; then
	echo "Requirements to start QEMU not met."
	echo "obc-qemu directory has to exist in same folder as the OBSW folder."
	echo "Please make sure to clone and build QEMU properly."
	exit
fi

directory=_bin
files=$(find . -type f -name "*.bin")

index=0
declare -a fileArray

for file in $files; do
printf "File $((index)): %s\r\n" "$file"
fileArray=("${fileArray[@]}" "$file")
index=$index+1
done;

echo "Select file with given index: "
read selection
if [[ -n ${selection//[0-9]/} ]]; then
echo "Selection contains letters!"
exit 
fi

if [[ $selection -lt  0 || $selection -gt ${#fileArray[@]} ]]; then
echo "Selection is invalid!"
exit
fi

target_binary=${fileArray[0]}
echo "Selected target file: $targetFile"

echo Launching QEMU:
echo

if [ "$input_arg_1" == "no-wait" ];then
	../obc-qemu/iobc-loader -- \
	-serial stdio -monitor none \
	-drive if=sd,index=0,format=raw,file=../obc-qemu/build/sd0.img \
	${target_binary} -s 
else
	../obc-qemu/iobc-loader -- \
	-serial stdio -monitor none \
	-drive if=sd,index=0,format=raw,file=../obc-qemu/build/sd0.img \
	${target_binary} -S -s 
fi

# Run this if you want to use telnet monitoring
# Run telnet localhost 55555 in separate console to connect.

#../obc-qemu/build/arm-softmmu/qemu-system-arm -M \
#isis-obc -monitor telnet:localhost:55555,server -serial stdio \
#-bios _bin/sam9g20/devel/sourceobsw-at91sam9g20_ek-sdram.bin 

# Old way which selected binary depending on certain strings.
# New way will expect selection from user!

# binaries=$(find $directory -type f -name "*.bin")
# target_binary=""
# target_binary_mission=""
# target_binary_devel=""

# set -- ${binaries}
# if [ $# == 0 ];then
# echo No valid binary found!
# fi

# echo $# Binaries found: 
# for binary in ${binaries}; do
#    echo ${binary}
#    if [[ ${binary} == *"devel"* ]];then
#        target_binary_devel=${binary}
#    elif [[ ${binary} == *"mission"* ]];then
#        target_binary_mission=${binary}
#    fi
#done

# if [ "${target_binary_mission}" != "" ];then
#    if [ "${input_arg_2}" == "mission" ];then
#        target_binary=$target_binary_mission
#        echo Mission binary $target_binary_mission selected
#    else
#        echo Devel binary $target_binary_devel selected
#        target_binary=$target_binary_devel
#        echo 
#    fi
#else
#    echo Devel binary selected
#    target_binary=$target_binary_devel
#fi

#if [ ${target_binary} == "" ]; then
#    echo No valid binary found
#    if [ $# == 1 ];then
#	echo Setting the one binary ${target_binary} found
#    	target_binary=binaries
#    fi
#    exit
#else 
#    echo Binary selected: 
#    echo ${target_binary}
#fi
#echo

