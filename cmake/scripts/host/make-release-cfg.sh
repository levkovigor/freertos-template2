#!/bin/sh
counter=0
cfg_script_name="cmake-build-cfg.py"
while [ ${counter} -lt 5 ]
do
    cd ..
    if [ -f ${cfg_script_name} ];then
        break
    fi
    counter=$((counter=counter + 1))
done

if [ "${counter}" -ge 5 ];then
    echo "${cfg_script_name} not found in upper directories!"
    exit 1
fi

build_generator=""
build_type="release"
builddir="build-Release-Host"
defines="HOST_BUILD=ON"

if [ "${OS}" = "Windows_NT" ]; then
	build_generator="MinGW Makefiles"
	os_fsfw="host"
	python="py"
# Could be other OS but this works for now.
else
    os_fsfw="linux"
	build_generator="Unix Makefiles"
	python="python3"
fi

echo "Running command (without the leading +):"
set -x # Print command 
${python} ${cfg_script_name} -o "${os_fsfw}" -g "${build_generator}" -b "${build_type}" \
        -l "${builddir}" -d "${defines}"
# set +x
