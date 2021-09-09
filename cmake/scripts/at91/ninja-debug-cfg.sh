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
os_fsfw="freertos"
builddir="build-Debug-AT91EK"
build_type="debug"
build_generator="ninja"
if [ "${OS}" = "Windows_NT" ]; then
    python="py"
# Could be other OS but this works for now.
else
    python="python3"
fi

echo "Running command (without the leading +):"
set -x # Print command 
${python} ${cfg_script_name} -o "${os_fsfw}" -g "${build_generator}" -l "${builddir}" \
    -b "${build_type}"
# Use this if commands are added which should not be printed
# set +x
