#!/bin/bash

# Kill background processes when script finishes
trap "trap - SIGTERM && kill -- -$$" SIGINT SIGTERM EXIT

echo "Performing SDRAM configuration"

obsw_folder=".."

if [ "${OS}" = "Windows_NT" ]; then
	gdb="arm-none-eabi-gdb.exe"
	jlink_cmd="JLinkGDBServerCL.exe -USB -device AT91SAM9G20 \
		-endian little -if JTAG -speed auto -noLocalhostOnly &"
	gdb_cmd="${gdb} -nx --batch -ex 'target remote localhost:2331' \
		-ex 'source ${obsw_folder}/at91/gdb/at91sam9g20-ek-sdram.gdb'"
else
	gdb="arm-none-eabi-gdb"
	jlink_cmd="JLinkGDBServerCLExe -USB -device AT91SAM9G20 -endian little -if JTAG \
		-speed adaptive -noLocalhostOnly &"
	gdb_cmd="${gdb} -nx --batch -ex 'target remote localhost:2331' -ex \
		'source ${obsw_folder}/at91/gdb/at91sam9g20-ek-sdram.gdb'"
fi

if ! command -v ${gdb} &> /dev/null
then
	echo "Command ${gdb} not found in path. Exiting.."
	exit 1
fi

echo "Launching JLink GDB Server.."
eval ${jlink_cmd}
@sleep 0.5
echo "Launching GDB script to enable SDRAM.."
eval ${gdb_cmd}
echo "Done"
