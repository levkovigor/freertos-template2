SOURCE On-Board Software
======

# General Information

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
software to the boards as well. The host build can be run locally on the host computers but
only Windows 10 and Ubuntu 20.04 were tested.
The QEMU image can be run on a Linux computer as well, but requireds QEMU installed as specified
in the QEMU documentation.

# Reference

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

# Prerequisites

1. Make installed. Part of [MSYS2 MinGW64](https://www.msys2.org/) on Windows.
2. Development board binaries: [GNU ARM Toolchain](https://xpack.github.io/arm-none-eabi-gcc/install/) 
   installed, hardware or QEMU set-up available. See the
   [Setting up prerequisites](#setting-up-prerequisites) section
3. On Windows: [MSYS2 MinGW64](https://www.msys2.org/) installed to have a Unix environment.
4. For QEMU: QEMU repository cloned and set up in same folder in which
   this repository was cloned

See separate [prerequisite](#prereq) chapter for more details

# Building the software

## Clone software

1. Create directory for OBSW.

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
   git submodule init
   git submodule sync
   git submodule update --recursive
   ```

You can now build the software for either the AT91 and iOBC targets
or for a host system.

##  Create OBSW binary for AT91 and iOBC

1. CMake is used to generate Makefiles to build the images. Therefore, install
CMake first. On Windows, Make needs to be installed. It is recommended to 
install MinGW64 as part of [MSYS2](https://www.msys2.org/) for this, see more
in prerequisites. Steps in Windows were performed in MinGW64 shell.

2. Create a build folder, run the `CMake` build generation command inside
that folder and then run the build command. On Windows, add `-G "MinGW Makefiles"` 
to the build command to avoid building for Visual Studio 2019.
You can supply the build type specifically, `Debug` will be the default.
To do this, supply the following arguments to the `CMake` build generation command:

 - Release: `-DCMAKE_BUILD_TYPE=Release`
 - Size: `-DCMAKE_BUILD_TYPE=MinSizeRel`
 - Release with Debug Info: `-DCMAKE_BUILD_TYPE=RelWithDebInfo`

If the boards are flashed for the first time, the SDRAM needs to be configured with
the following command

```sh
./sdramCfg
```

It is recommended to use Eclipse to flash the boards conveniently.
A list of all important combinations will be shown for the Debug configuration.
Please note that all commands here can be run conveniently by using 
the shell scripts provided in the `cmake/scripts` folder.

### Build for the AT91-EK

Can be loaded into SDRAM directly or to NAND-Flash 0x40000 to be loaded
by bootloader.

```sh
mkdir build-Debug-AT91EK && cd build-Debug-AT91EK
cmake ..
cmake --build . -j
```

### Build for the iOBC

Load at NOR-Flash 0x20000 when using the custom bootloader or NOR-Flash 0xA000
when using ISIS bootloader.

```sh
mkdir build-Mission-iOBC && cd build-Mission-iOBC
cmake -DBOARD_IOBC=ON ..
cmake --build . -j
```
   
### Build bootloaders for the AT91-EK

First stage bootloader.
Load at NAND-Flash position 0x0, and edit sixth ARM vector
to contain binary size (SAM-BA recommended). More information in AT91 README.

```sh
mkdir build-Mission-BL-AT91EK && cd build-Mission-BL-AT91EK
cmake -DBOOTLOADER=ON -DCMAKE_BUILD_TYPE=MinSizeRel .. 
cmake --build . -j
```

Second stage bootloader.
Load at NAND-Flash position 0x20000, more information in AT91 README

```sh
mkdir build-Debug-BL2-AT91EK && cd build-Debug-BL2-AT91EK
cmake -DBOOTLOADER=ON -DBL_STAGE_TWO=ON -DCMAKE_BUILD_TYPE=Debug .. 
cmake --build . -j
```

### Build bootloader for the iOBC

```sh
mkdir build-Mission-BL-iOBC && cd build-Mission-BL-iOBC 
cmake -DBOOTLOADER=ON -DBOARD_IOBC=ON -DAT91_NO_FREERTOS_STARTER_FILE=ON -DCMAKE_BUILD_TYPE=Release .. 
cmake --build . -j
```

## Starting QEMU

Command to start QEMU (inside sourceobsw folder). Please note this only works if the QEMU 
repository was cloned and built inside the same folder the OBSW was cloned.
```sh
./StartQEMU.sh
``` 
   
## Build Host Software

Perform the following steps to build the hosted software

### Windows

Install [MSYS2](https://www.msys2.org/) if not done so already. See [prerequisites](#prereq) chapter
for more information.
Now you can run the following commands in the `sourceobsw` folder to build the software

```sh
mkdir build-Debug-Host && cd build-Debug-Host
cmake .. -G "MinGW Makefiles"
cmake --build . -j
```

### Linux

Run the following command in the `sourceobsw` folder to build the software
with the hosted OSAL. You can supply `-DOS_FSFW=linux` to the `cmake ..` command to build with the 
Linux OSAL instead

```sh
mkdir build-Debug-Host && cd build-Debug-Host
cmake ..
cmake --build . -j
```

# Build Configurations and testing of Flight Software

The provided TMTC has separate [instructions](https://git.ksat-stuttgart.de/source/tmtc)
It is possible to chose between serial communication via RS232 and Ethernet 
communication with UDP datagrams. For the serial communication, a USB to female 
RS232 cable can be used (or UART jumper wires..).

# <a name="prereq"></a> Setting up prerequisites

## Windows: Installing and setting up MinGW64

1. Install [MSYS2](https://www.msys2.org/).
2. Run the following commands in MinGW64

   ```sh
   pacman -Syuuu
   pacman -S git mingw-w64-x86_64-gcc mingw-w64-x86_64-gdb mingw-w64-x86_64-make mingw-w64-x86_64-cmake
   ```

   Alternatively, you can use

   ```sh
   pacman -S mingw-w64-x86_64-toolchain
   ```

   to install `gdb`, `gcc` and `mingw32-make` at once. It is recommended to set up `alias`es in 
   the `.bashrc` file to nagivate to the working directory quickly.

3. Run `git config --global core.autocrlf true` if you like to use MinGW64 `git` as well

4. Open a new path setter file called `source_path_set.sh` file

   ```sh
   cd ~
   nano source_path_set.sh
   ```

   Add the following line in thge file to MinGW64.
   This is just an example, correct the path for your use-case with the correct
   version and user name!

   ```sh 
   export PATH="/c/Users/Robin/AppData/Roaming/xPacks/@xpack-dev-tools/arm-none-eabi-gcc/10.2.1-1.1.2/.content/bin":$PATH
   ```
   
   Now you can run
   
   ```sh
   source source_path_set_sh
   ```
   
   to add the path to the MinGW64 path temporarily.
   Alternatively, you can put this `export` command in the `.profile` and `.bashrc` file
   so the path is always added. However, the approach shown above keeps the path clean.

## Windows: Installing and setting up the ARM Toolchain
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
   
4. It is not recommendend anymore to put the path containing the binaries into the Windows system 
   path because there can be nameclashes with Windows dynamic linked libraries. Instead, it is 
   recommended to add them to the MinGW64 path temporarily like shown above before setting a a 
   `CMake` build environment. `CMake` will then cache the toolchain path, so the Windows environment 
   path stays clean.
   
If you don't want to install nodejs you may go with the 
[four-command manual installation](https://xpack.github.io/arm-none-eabi-gcc/install/#manual-install). 

## Linux: Install C++ buildchain on Linux

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

# Project Specific Information

There are some important differences of this project compared to the project files and configuration 
provided by ISIS. Some important differences will be documented and listed here. It should be noted 
that memory allocation is only performed during start-up and was carefully avoided during run-time 
to avoid associated problems like non-deterministic behaviour and memory fragmentation in the heap.

### C++
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

### FreeRTOS

It is possible to use a newer version of FreeRTOS. The ISIS libraries still use the API of 
FreeRTOS 7.5.3. A newer FreeRTOS can be used as long as the old API calls are still provided and 
forwarded to the new API. The function implementation is contained within the `isisAdditions.c` 
source file while the ISIS change log in the doc folder contains more specific information.

Please note that the configuration option `configUSE_NEWLIB_REENTRANT` was set to one as well to 
ensure that newlib nano can be used in a thread-safe manner. Functon implementations for 
`__malloc_lock` and `__malloc_unlock` were provided as well to ensure thread-safety when using 
newlib nano with FreeRTOS. This project also uses the `heap4.c` FreeRTOS memory management scheme.

### Pre-emptive scheduling

ISIS default FreeRTOS configuration uses a cooperative scheduler and their documents specify that 
this is due "higher requirements to data contention management". It is not exactly known what 
this means, but there have been no issues with using a pre-emptive scheduler so far.

### Newlib Nano

Newlib Nano is used as the a library for embedded systems. This reduces the binary size as well

