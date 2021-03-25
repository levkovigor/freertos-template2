#!/bin/sh
counter=0
while [ ${counter} -lt 5 ]
do
	cd ..
	if [ -f "cmake_build_config.py" ];then
		break
	fi
	counter=$((counter=counter + 1))
done

if [ "${counter}" -ge 5 ];then
	echo "cmake_build_config.py not found in upper directories!"
	exit 1
fi

build_generator=""
os_fsfw="freertos"
builddir="MinSizeRel-iOBC"
defines="BOARD_IOBC=ON"

if [ "${OS}" = "Windows_NT" ]; then
	build_generator="MinGW Makefiles"
# Could be other OS but this works for now.
else
	build_generator="Unix Makefiles"
fi

echo "Running command (without the leading +):"
set -x # Print command 
python3 cmake_build_config.py -o "${os_fsfw}" -g "${build_generator}" -b "size" \
         -d "${defines}" -l "${builddir}"
# Use this if commands are added which should not be printed
# set +x
