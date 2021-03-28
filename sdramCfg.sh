#!/bin/sh
echo "Performing SDRAM configuration"
trap "trap - SIGTERM && kill -- -$$" SIGINT SIGTERM EXIT

if [ "${OS}" = "Windows_NT" ]; then
	JLinkGDBServerCL.exe -USB -device AT91SAM9G20 -endian little -if JTAG \
	-speed auto -noLocalhostOnly &
	@sleep 1
	arm-none-eabi-gdb.exe -nx --batch -ex 'target remote localhost:2331' -ex \
	'source at91/gdb/at91sam9g20-ek-sdram.gdb'
else
	JLinkGDBServerCLExe -USB -device AT91SAM9G20 -endian little -if JTAG \
	-speed adaptive -noLocalhostOnly &
	@sleep 1
	arm-none-eabi-gdb -nx --batch -ex 'target remote localhost:2331' -ex \
	'source at91/gdb/at91sam9g20-ek-sdram.gdb'
fi

