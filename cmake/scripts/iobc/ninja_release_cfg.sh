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
build_type="release"
builddir="build-Mission-iOBC"
defines="BOARD_IOBC=ON"
build_generator="Ninja"

echo "Running command (without the leading +):"
set -x # Print command 
python3 cmake_build_config.py -o "${os_fsfw}" -g "${build_generator}" -b "${build_type}" \
        -d "${defines}" -l "${builddir}"
# set +x