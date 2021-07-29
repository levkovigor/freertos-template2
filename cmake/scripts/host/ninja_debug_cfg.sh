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
build_type="debug"
builddir="build-Debug-Host"
defines="HOST_BUILD=ON"
build_generator="Ninja"
	
if [ "${OS}" = "Windows_NT" ]; then
	os_fsfw="host"
# Could be other OS but this works for now.
else
    os_fsfw="linux"

fi

echo "Running command (without the leading +):"
set -x # Print command 
python3 cmake_build_config.py -o "${os_fsfw}" -g "${build_generator}" -b "${build_type}" \
        -l "${builddir}" -d "${defines}"
# set +x
