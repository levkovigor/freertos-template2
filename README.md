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

- iOBC
- AT91SAM9G20-EK
- Hosted (Linux and Windows)
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
[Project specific information](#project-specific-information)<br>

**Specific documentation**<br>
[Installing and setting up Eclipse](doc/README-eclipse.md#top)<br>
[Developers documentation](doc/README-dev.md#top)<br>
[AT91SAM9G20 getting started](doc/README-at91.md#top)<br>
[Flatsat getting started](doc/README-flatsat.md#top)<br>
[Common TMTC commands](doc/README-tmtc.md#top)<br>
[Test summary and progress](doc/README-test.md#top)<br>
[QEMU getting started](doc/README-qemu.md#top)<br>
[Hosted build getting started](doc/README-hosted.md#top)<br>

## Prerequisites
1. Make installed
2. Development board binaries: [GNU ARM Toolchain](https://xpack.github.io/arm-none-eabi-gcc/install/) 
   installed, hardware or QEMU set-up available. See the
   [Setting up prerequisites](#setting-up-prerequisites) section
3. For Host build: On Windows, GCC toolchain (MinGW64)
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

   General command for Host systems (Windows or Linux):
   ```sh
   make -f Makefile-Hosted all
   ```
   
   General command for Linux:
   ```sh
   make -f Makefile-Linux all
   ```
   
   Command to start QEMU (inside sourceobsw folder). Please note this
   only works if the QEMU repository was cloned and built inside the same folder
   the OBSW was cloned.
   ```sh
   ./StartQEMU.sh
   ``` 
   
6. The Linux and Hosted binaries can be run directly via command line or by executing them on the 
   host. The development board binaries have to be flashed with with J-Link/SAM-BA for the AT91 and 
   the `sdramCfg` make target needs to be run first once per AT91 power cycle before flashing the 
   SDRAM. Refer to respective instructions for more details.

### Build Configurations and testing of Flight Software

Compilation can be sped up by providing the -j parameter.
On windows, wsl must be added before make, if tools like MSYS2 or Windows Build
Tools are not installed. It is recommended to set up Eclipse instead of using the command line to 
build the software, as this allows for much more convenient development and debugging.
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
The code needs to be compiled for the ARM target system and we will use the
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
   These are usually located in C:\Users\<...>\AppData\Roaming\xPacks\@gnu-mcu-eclipse\arm-none-eabi-gcc\<version>\.content\bin .
   Alternatively, if you want to keep the environment and the path clean, add it temporarily 
   with `SET PATH=%PATH%;c:\pathtotoolchain` before each debugging session(command can be put 
   inside a bash script).  
   
If you don't want to install nodejs you may go with the 
[four-command manual installation](https://xpack.github.io/arm-none-eabi-gcc/install/#manual-install). 

### Linux: Install C++ buildchain on Linux

Install the [GNU ARM toolchain](https://xpack.github.io/arm-none-eabi-gcc/install/)
like explained above, but for Linux, windows build tools not required.

On Linux, the a path can be added to the system variables by adding
`export PATH=$PATH:<..../@gnu-mcu-eclipse/arm-none-eabi-gcc/<version>/.content/bin>`
to the `.profile` or `.bashrc` file. Alternatively, if you want to keep the environment and the 
path clean, add it temporarily with `export PATH=$PATH:<pathToToolchainBinaries`.  
   
To install general buildtools for the linux binary, run:
```sh
sudo apt-get install build-essential
```

## Project Specific Information

There are some important differences of this project compared to the project files and configuration 
provided by ISIS. Some important differences will be documented and listed here. It should be noted 
that memory allocation is only performed during start-up and was carefully avoided during run-time 
to avoid associated problems like non-deterministic behaviour and memory fragmentation in the heap.

#### C++
C++ is used in this project. To allow this, some important changes in the linkerscript files and 
the start up files were necessary. The most important change includes specifying `.fini`, `.init`,
`.preinit_array`, `.init_array` and `.fini_array` sections. In the startup 
file `__libc_init_array` is called before branching to main to ensure all global constructors 
are called.

### FSFW

This project uses the FSFW flight-proven small satellite framework. The framework provides many 
components and modules to easy development. Examples include an object manager, an abstraction 
layer for FreeRTOS, a PUS stack for TMTC commanding using the ECSS PUS standard and a lot more. 
More information can be found at the [FSFW](https://egit.irs.uni-stuttgart.de/fsfw/fsfw) website.

#### FreeRTOS

It is possible to use a newer version of FreeRTOS. The ISIS libraries still use the API of 
FreeRTOS 7.5.3. A newer FreeRTOS can be used as long as the old API calls are still provided and 
forwarded to the new API. The function implementation is contained within the `isisAdditions.c` 
source file while the ISIS change log in the doc folder contains more specific information.

Please note that the configuration option `configUSE_NEWLIB_REENTRANT` was set to one as well to 
ensure that newlib nano can be used in a thread-safe manner. Functon implementations for 
`__malloc_lock` and `__malloc_unlock` were provided as well to ensure thread-safety when using 
newlib nano with FreeRTOS. This project also uses the `heap4.c` FreeRTOS memory management scheme.

#### Pre-emptive scheduling

ISIS default FreeRTOS configuration uses a cooperative scheduler and their documents specify that 
this is due "higher requirements to data contention management". It is not exactly known what 
this means, but there have been no issues with using a pre-emptive scheduler so far.

#### Newlib Nano

Newlib Nano is used as the a library for embedded systems. This reduces the binary size as well

