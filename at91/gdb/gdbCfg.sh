#!/bin/bash
arm-none-eabi-gdb.exe -nx --batch -ex 'target remote localhost:2331' -ex 'source at91sam9g20-ek-sdram.gdb' &
echo "SDRAM configured successfully" &
$SHELL