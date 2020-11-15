SOURCE On-Board Software
======

## General Information
More general information of the project can be found
on the [KSat website](https://www.ksat-stuttgart.de/en/our-missions/source/).

The SOURCE On-Board Software will be run on the iOBC which uses the SAM9G20
SoC. The software is based on the Flight Software Framework by the IRS.
It is recommended to use the [Python TMTC commander](https://git.ksat-stuttgart.de/source/tmtc)
to test TMTC handling. Packets can be sent to the Linux version by using the UDP communication 
interface of the commander.

Used development boards and environments:

- STM32 Nucleo-144 H743ZI
- AT91SAM9G20-EK
- Linux
- QEMU

Additional Note: The ISIS library is not public because
it is not open source. Those libraries to be added manually and the includes
and source files have to be setup and included accordingly!

The device specific documentation contains information on how to flash the built
software to the boards as well. The Linux build can be run locally on the host computer.
The QEMU image can be run on the host computer as well, but required QEMU installed as specified
in the QEMU documentation.

## Reference
[Prerequisites](#prerequisites)<br>
[Building the software](#building-the-software)<br>
[Setting up prerequisites](#setting-up-prerequisites)<br>

**Specific documentation**<br>
[Installing and setting up Eclipse](doc/README-eclipse.md#top)<br>
[Developers documentation](doc/README-dev.md#top)<br>
[AT91SAM9G20 getting started](doc/README-at91.md#top)<br>
[Flatsat getting started](doc/README-flatsat.md#top)<br>
[Common TMTC commands](doc/TMTC.md#top)<br>
[QEMU getting started](doc/README-qemu.md#top)<br>
[Linux and Unittest getting started](doc/README-linux.md#top)<br>
[STM32 getting started](stm32/README-stm32.md#top)<br>

## Prerequisites
1. Make installed
2. Development board binaries: [GNU ARM Toolchain](https://xpack.github.io/arm-none-eabi-gcc/install/) 
   installed, hardware or QEMU set-up available. See the [Setting up prerequisites](#setting-up-prerequisites)<br>
   section
3. For Linux and Unit Test binaries: Linux OS or virtual Machine, at leastfull C++
   build chain installed
4. For QEMU: QEMU repository cloned and set up in same folder in which 
   this repository was cloned

## Building the software
### Clone software and create OBSW binary

1. Create directory for OBSW (e.g. with mkdir Source_OBSW).
Note that make and git are required (installation guide below)
2. Clone OBSW into this directory
   ```sh
   git clone https://git.ksat-stuttgart.de/source/sourceobsw.git
   ```
3. Switch branch to desired branch (if not master branch)
   ```sh
   git checkout <branch>
   ```
4. Initialize and update all submodules
   ```sh
   git submodule update --init --recursive
   ```

5. Run Makefile to create binaries. If running on linux and .exe ending is
   problematic, supply WINDOWS=1 additionally. If running on windows and  
   build tools like MSYS2 or Windows Build Tools are not installed, add
   wsl be fore make.
   Please note that there are different build options and configuration parameters
   for the make file available. An explanation and how to set up the Eclipse IDE
   for these configurations will be provided in a separate chapter.
   There are following targets available:
   
   1. all and debug to build the debug binaries
   2. mission to build the release binary

   General command for AT91:
   ```sh
   make all
   ```
   General command for the iOBC:
   ```sh
   make IOBC=1 all
   ```

   General command for STM32:
   ```sh
   make -f Makefile-STM32 all
   ```
   General command for Linux:
   ```sh
   make -f Makefile-Linux all
   ```
   General command for the Unit Test:
   ```sh
   make -f  Makefile-Unittest all
   ```
   Command to start QEMU (inside sourceobsw folder). Please note this
   only works if the QEMU repository was cloned and built inside the same folder
   the OBSW was cloned.
   ```sh
   ./StartQEMU.sh
   ``` 
6. The linux and unit test binaries can be run directly
   in the terminal or with eclipse. The development board binaries have to be flashed with
   OpenOCD for STM32 or with J-Link/SAM-BA for AT91.
   Refer to respective instructions.

### Build Configurations and testing of Flight Software

Compilation can be sped up by providing the -j parameter.
On windows, wsl must be added before make, if tools like MSYS2 or Windows Build
Tools are not installed. In any case, WINDOWS=1 has to be supplied.
It is recommended to set up Eclipse instead of using the command line to build
the software, as this allows for much more convenient development and debugging.
For developers unfamiliar with Eclipse, it is recommended to read the
[Eclipse setup guide](doc/README-eclipse.md#top).

Following make targets are available:
- sdramCfg: Configure AT91 SDRAM on start-up. Required after each restart.
- clean: Clean the dependencies, binaries and includes of current active build
  configurationand Communication Interface (Serial RS232 or UDP Ethernet)
- hardclean: Clean the three mentioned folders for all systems and interfaces
- cleanbin: Clean all binaries
- debug: Additional FSFW debug messages
- virtual: Virtualized software interfaces replace real connected hardware
- mission: Optimized build for mission. Not good for debugging.

Example call to build mission build:
```sh
make mission
```

Example call to build debug build in windows for the iOBC:
```sh
make debug WINDOWS=1 IOBC=1
```

The provided TMTC has separate [instructions](https://git.ksat-stuttgart.de/source/tmtc)
It is possible to chose between serial communication via RS232 and Ethernet 
communication with UDP datagrams. For the serial communication, a USB to female 
RS232 cable can be used (or UART jumper wires..).

## Setting up prerequisites

### Windows: Installing and setting up the ARM Toolchain
The installation on windows is very similar, also using the
[GNU ARM Toolchain](https://xpack.github.io/arm-none-eabi-gcc/install/).

1. Install NodeJS LTS. Add nodejs folder (e.g. "C:\Program Files\nodejs\")
   to system variables. Test by running `npm --version` in command line
2. Install [XPM](https://www.npmjs.com/package/xpm)
   ```sh
   npm install --global xpm
   ```

3. Install gnu-arm Toolchain for Eclipse (version can be specified)
   ```sh
   xpm install --global @xpack-dev-tools/arm-none-eabi-gcc@latest
   xpm install --global @xpack-dev-tools/windows-build-tools@latest
   ```
   
   Optional for STM32 build
   ```sh
   xpm install --global @xpack-dev-tools/openocd@latest
   ```
   
4. Add arm-none-eabi-gcc binary location in the xPack folder to system variables. 
   These are usually located in C:\Users\<...>\AppData\Roaming\xPacks\@gnu-mcu-eclipse\arm-none-eabi-gcc\<version>\.content\bin
   
If you don't want to install nodejs you may go with the 
[four-command manual installation](https://xpack.github.io/arm-none-eabi-gcc/install/#manual-install). 

### Linux: Install C++ buildchain on Linux if using Linux
Will follow. This will install all required tools for C++ compilation and make.

Install the [GNU ARM toolchain](https://xpack.github.io/arm-none-eabi-gcc/install/)
like explained above, but for Linux, windows build tools not required.

On Linux, the a path can be added to the system variables by adding
`export PATH=$PATH:<..../@gnu-mcu-eclipse/arm-none-eabi-gcc/<version>/.content/bin>
to the `.profile` or `.bashrc` file.
   
To install general buildtools for the linux binary, run:
```sh
sudo apt-get install build-essential
```

