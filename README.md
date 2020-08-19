Source On-Board Software
======

## General Information
The SOURCE On-Board Software will be run on the iOBC which uses the SAM9G20
chip. It is based on the Flight Software Framework by the IRS.

Used development boards and environments:

- STM32 Nucleo-144 H743ZI
- AT91SAM9G20-EK
- Linux
- QEMU

## Reference
[Prerequisites](#prerequisites)<br>
[Building the software](#building-the-software)<br>
[Setting up prerequisites](#setting-up-prerequisites)<br>
[Developers information](#developers-information)<br>
[git and doxygen](#git-and-doxygen)<br>
[C++](#cpp)<br>

**Board and environment specific introduction**<br>
[AT91SAM9G20 getting started](sam9g20/README-at91.md#top)<br>
[QEMU getting started](doc/README-qemu.md#top)<br>
[Flatsat getting started](doc/README-flatsat.md#top)<br>
[Linux and Unittest getting started](doc/README-linux.md#top)<br>
**Deprecated or not working at the moment**<br>
[STM32 getting started](stm32/README-stm32.md#top)<br>

## Prerequisites
1. Make installed
2. Development board binaries: GNU ARM Toolchain installed, hardware or QEMU set-up available.
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
4. Import Flight Software Framework (FSFW)
   ```sh
   git submodule init
   git submodule update
   ```

5. Run Makefile to create binaries. If running on linux and .exe ending is
   problematic, supply WINDOWS=1 additionally. If running on windows and  
   build tools like MSYS2 or Windows Build Tools are not installed, add
   wsl be fore make.
   Please note that there are different build options and configuration parameters
   for the make file available. An explanation and how to set up the Eclipse IDE
   for these configurations will be provided in a separate chapter.

   General command for AT91:
   ```sh
   make all
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
   only works if one binary file is located in \_bin for now 
   (the shell script finds all files named *.bin inside that folder)
   ```sh
   ./StartQEMU.sh
   ``` 
6. The linux and unit test binaries can be run directly
   in the terminal or with eclipse. The development board binaries have to be flashed with
   OpenOCD for STM32 or with J-Link/SAM-BA for AT91.
   Refer to respective instructions.

### Build Configurations and testing of Flight Software

Don't forget the use compile optimization by providing the -j parameter.
On windows, wsl must be added before make, if tools like MSYS2 or Windows Build
Tools are not installed.

Following make targets are available:
- sdramCfg: Configure AT91 SDRAM on start-up. Required after each restart.
- clean: Clean the dependencies, binaries and includes of current system (STM32 or AT91)
  and Communication Interface (Serial RS232 or UDP Ethernet)
- hardclean: Clean the three mentioned folders for all systems and interfaces
- cleanbin: Clean all binaries
- debug: Additional FSFW debug messages
- virtual: Virtualized software interfaces replace real connected hardware
- mission: Optimized build for mission. Not good for debugging.

Example call to build mission build:
```sh
make mission
```

Example call to build debug build on windows:
```sh
make debug WINDOWS=1
```
Currently, there are two binary folders: One for the development binaries
and one for the mission binaries. For the dependency and object folders, four different combinations are possible
(two for the different comm interfaces and two for the binary types).
Thus, if a different binary is needed, e.g. when switching from devel build to mission build,
and all object files are already available, it is sufficient to clean the binaries with
the cleanbin target and then rebuild the target build. Make will recognize that the object files are already available and link the new requires binary.

The provided TMTC has separate [instructions](https://git.ksat-stuttgart.de/source/tmtc)
It is possible to chose between serial communication via RS232 and Ethernet communication with UDP datagrams. For the serial communication, a USB to female RS232 cable can be used (or UART jumper wires..).

Instructions to set up Eclipse for these build targets are provided in the AT91 Getting Started chapter.

## Setting up prerequisites

### Windows: Installing and setting up the ARM Toolchain
The installation on windows is very similar, also using the [GNU ARM Toolchain](https://xpack.github.io/arm-none-eabi-gcc/install/).

1. Install NodeJS LTS. Add nodejs folder (e.g. "C:\Program Files\nodejs\")
   to system variables. Test by running npm in command line
2. Install [XPM](https://www.npmjs.com/package/xpm)
   ```sh
   npm install --global xpm
   ```

3. Install gnu-arm Toolchain for Eclipse (version can be specified)
   ```sh
   xpm install --global @xpack-dev-tools/arm-none-eabi-gcc@latest
   xpm install --global @xpack-dev-tools/windows-build-tools@latest
   xpm install --global @xpack-dev-tools/openocd@latest
   ```
4. Add arm-none-eabi-gcc binary location in the xPack folder to system variables. These are usually located in C:\Users\<...>\AppData\Roaming\xPacks\@gnu-mcu-eclipse\arm-none-eabi-gcc\<version>\.content\bin

If you don't want to install nodejs you may go with the [four-command manual installation](https://xpack.github.io/arm-none-eabi-gcc/install/#manual-install). 

### Linux: Install C++ buildchain on Linux if using Linux
Will follow. This will install all required tools for C++ compilation and make.

Install the [GNU ARM toolchain](https://xpack.github.io/arm-none-eabi-gcc/install/)
like explained above, but for Linux, windows build tools not required.

To install general buildtools for the linux binary, run:
```sh
sudo apt-get install build-essential
```

## Developers Information

Developing software on microcontrollers requires a lot of software tools and is generally
more complicated than Desktop App Development. Tools like QEMU or docker can/will be used
to simplify the proccess as they provide virtualization and encapsulation of the embedded
environment without the need of countless software tools and real hardware.
However, testing on the real hardware will still be very important to ensure
the software runs successfully on the target environment (iOBC by ISIS) without any issues.
The following instructions contain all required tools to set up a decent development environment on Windows to work with the hardware.
The steps will be easier on Linux and the Unit Test and Linux binaries can only
be used in a Linux Virtual Machine or full Linux OS installation, so it is
worth considering setting up dual boot with linux or setting up a virtual machine.
Not all steps might be necessary depending on the experiences and already available tools of a
new developer.

### Installation Eclipse for C/C++ Developers on Windows

1. Install JDK if not installed yet. Eclipse requires [Java SE Platform (JDK)](https://www.oracle.com/technetwork/java/javase/downloads/index.html)
2. Install [Eclipse for C/C++ Developers](https://www.eclipse.org/downloads/packages/release/2019-03/r/eclipse-ide-cc-developers)
3. Go to Menu -> Help -> Eclipse Marketplace and search and install GNU MCU Eclipse

### Required steps to build flight software in Eclipse for C/C++ Developers

1. In Eclipse, import the souceobsw folder as a Makefile project
2. Rightclick on sourceobsw, go to MCU and ensure the Arm Toolchain is found (xPack should be found automatically)
3. Go to C/C++ Build -> Settings -> Toolchain, make sure the toolchain path is set correctly and press apply.
Toolchain binaries should appear in includes in the folder structure (important for indexer)
4. Rightclick on sourceobsw, properties, and use following build settings:
   GNU Toolchains for Ubuntu are needed for this !
   Example build, using windows build tools
   ```sh
   > make all
   ```
5. Build acceleration can be turned on by going to behaviour options and
   enabling parallel build.


### Installation Linux Subsystem (WSL) or any other command line program for windows

A command line program like WSL (Ubuntu Subsystem) or MSYS2 can be useful
because of used tools make and useful for tools like git.

WSL Can be installed by following the [installation instructions](https://docs.microsoft.com/de-de/windows/wsl/install-win10)
for Ubuntu Subsystem.
An IDE like Eclipse for C/C++ is very useful and has been chosen for this project.

Alternatively, make and git can also be used in windows. For make, the
windows build tools must be installed (can be done with the xpm packet manager).

1. For installaton on windows, install Linux Subsystem (WSL in Windows Store)
or similar command line programm (z.B. MSYS2/MinGW)
2. Install git
	```sh
 	sudo apt-get install git
	```
3. Install Make
	```sh
  	sudo apt-get install make
  	```
3. Install editor programm like vim or atom. Notepad++ can be used to
but needs to be included in the Windows Environment Variables. After that,
notepad++.exe can be called. (or an alias like np='notepad++.exe' can be used)
	```sh
 	sudo apt-get install vim
 	sudo apt-get install atom
	```
4. An alias (shortcut) in ubuntu is very useful to navigate to the
   windows and/or development directories quickly. Any editor can be used to create an alias
```sh
 > cd ~
 > nano .bashrc
````
Add new line
```sh
 > alias shortcut="cd /mnt/c/Users/..."
````
Restart command line programm and test the alias by typing
```sh
> shortcut
````
5. Update everything
```sh
  > sudo apt-get update
  > sudo apt-get
```

### General Structure OBSW which is included in binary file for AT91

- General Sequence: Compile and link software -> flash generated binary to board -> Run or Debug
- Core files: Makefile, linker script, board startup assembler file and main file
- Makefile: Compile and link software with ARM Toolchain
- Linker Script: Used by makefile, maps input files to output file, control memory layout of binary file
- Board Startup file: Perform absolutely necessary configuration at start-up before branching to main.cpp
- Main: Configure board and start RTOS scheduler. Calls init_mission() which contains all missions tasks
- mission: Contains mission specific code which uses the Flight Software Framework (FSFW)
- framework: Contains FSFW
- config: Mission specific configuration of FSFW and additions. Contains object factory and IDs
- board/environment specific folders.

### Additional tools

- doc: General documentation, doxygen
- generators: Exporter for objects, events, return values and packet definitions
- tmtc: Python TMTC script which is command line configured by now. PyCharm configuration provided.

## git and doxygen

#### git basics

[Complicated git reference manual](https://git-scm.com/book/en/v2)<br>
[Better git reference manual](https://rogerdudler.github.io/git-guide/)<br>
General sequence to update:
1. Please note that framework changes need to be commited and pushed automatically while being in the ksat_branch.
git pull is not strictly necessary but ensures that any changes are included before pushing own content
```sh
git add .
git status
git commit -m "<commit message>"
git pull
git push
````
2. Useful commands
```sh
git checkout <branch>
git diff (--staged)
git log
git merge <branch to be merged into current branch>
git remote update origin --prune
```
3. Submodule Operations
```sh
git submodule init
git submodule update
git submodule sync
```
4. Create Tag for important branches/merges and push them to gitlab
```sh
git tag -a <VersionTag> -m <VersionMessage>
git push origin tag <VersionTag>
````
5. Create new branch (personal branch like <name>\_branch or feature
branch <feature>\_featureDetails). git checkout -b copies the state of the
current branch
```sh
git checkout -b <new branch name>
git merge <any other branches to include>
```
6. If you worked in wrong branch accidentally and want to apply changes to
another branch
```sh
git stash
git checkout <target branch>
git stash apply
```
7. Rename branch and remote branch
```sh
> git checkout <target branch>
git branch -m new-name
git push origin -u new-name
git push -d origin old-name
````
8. Delete branch and remote branch
```sh
git branch -d branch
git push origin -d remote_branch
```
9. Add new submodule (= other repository) to repository.
Run normal submodule sequence (3.) after this.
```sh
git submodule add <repository address> <folder name>
```
10. Revert commit but keep changes (e.g. to stash them and apply them somewhere
else)
```sh
git reset --soft HEAD~<numberOfCommitsToGoBack>
```

- git checkout is used to switch the currently used branch.
- git diff lists the differences of current branch to last local commit.
  Use --staged if new content was already added.
- git log lists the last few commits.

The submodule commands are useful because the FSFW is integrated as a submodule.
Generally, a new branch is created for each new user and for each new feature.
Name convention:


#### git branching models

Generally, there are guidelines on how to use git and how to name branches.
For the sourceobsw, the following guideline can be used:
- lastname/master as personal branch
- lastname/feature/featurename as feature branch
- lastname/test/testname as a test branch

The feature branch is merged into the master once it has been tested thoroughly.
If work was done in wrong branch accidentaly, use git stash, git apply or git
pop to move changes to different branch (see 6.). If there are wrong commits,
consider 10. Merge requests can be performed with a GUI in GitLab.

The FSFW is included as a submodule. The devel branch generally points to the
main repository on [egit](https://egit.irs.uni-stuttgart.de/fsfw/fsfw), provided by the IRS.
The main repository was forked for [KSat](https://egit.irs.uni-stuttgart.de/KSat).
The master of this fork will always point to the main repository, while other
branches can be used as feature branches.
If any changes in the framework a required for mission features, create a new
branch in the fork for this new feature. An issue and merge request can then be issued.

The ideal git development cycle:
1. Create an issue for the feature that is being implemented
2. Develop the feature
3. Create a pull request
4. Other developers can comment the code in the pull request
5. The pull request is approved and merged into the master.

### Code Documentation with Doxygen

Doxygen was used as a tool to generate the documentation. PDFs can not be produced yet because of a doxygen bug.
The documentation can be accessed by finding the index.html file in doc/doxy/html/
To generate new documentation on Windows, following steps have to be taken:

1. Install [doxygen](http://www.doxygen.nl/download.html)
2. Install [graphviz](https://graphviz.gitlab.io/_pages/Download/Download_windows.html)
3. Add the graphviz binary folder to PATH/system variables
4. Start doxyfile OPUS.doxyfile located in doc/doxy with the doxywizard gui to configure the documentation
5. Generate documentation gui or run doxyfile with doxygen

## <a name="cpp"></a>C++
### Coding Standards and C++ version
The framework uses the C++11 version which offers a lot of useful features of modern C++ and
the mission code uses C++17 for now. The indexer can be set up to index with C++17:

- Right-click the sourceobsw project and go to Properties->C/C++ General->Preprocessor Includes->CDT ARM Cross and enable the global provider.
- After that change the global provider settings by clicking on workspace settings and going to Discovery->CDT Arm Cross
- Put into the command line:
```sh
${COMMAND} ${FLAGS} ${cross_toolchain_flags} -std=c++17 -E -P -v -dD "${INPUTS}"
```

There are a lot of coding standards resources for the C++ language.
The first resource usually is the [C++ coding standard](https://isocpp.org/wiki/faq/coding-standards)
information page. There are many coding standards, for example by

- [ESA](http://www.esa.int/TEC/Software_engineering_and_standardisation/TECT5CUXBQE_0.html) (website download doesn't work..),
- [C++ core guidelines](http://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines) by the C++ creator himself
- [FSFW coding guidelines, not public yet](https://egit.irs.uni-stuttgart.de/fsfw/Coding_Guidelines)
- [JSF guidelines](http://stroustrup.com/JSF-AV-rules.pdf) by Lockheed Martin
- [Google C++ guidelines](https://google.github.io/styleguide/cppguide.html)


Because this code uses the FSFW, coding guidelines are based on the FSFW guidelines.
A general list of adaptations and style guidelines will be provided:

- Try to group includes logically, listing mission includes first, then configuration  
  includes, framework includes, driver includes and lastly standard library includes.
  Try to include in the source file(.cpp) if possible.
- Keep columns width to 80. The column width can be set higher in Eclipse by going
  to Window->C/C++->CodeStyle->Formatter  and defining a custom profile built on top of
  the K&R profile with LineWrapping->MaximumLineWidth set to the desired value
  (default value is 80). This doesn't have to be followed stricly
  but adhering to this column width consistently helps with code readability.
- Type definitions for built-in types have a \_t appended at the end
- Prefer explicit types like uint8_t, uint16_t, uint32_t
- Use all upper case letters with words seperated by underscores for constants
  and enum members.
  ```cpp
  typedef uint8_t my_type_t
  static const my_type_t MY_CONSTANT = 1
  enum class MyTestEnum: uint8_t {
	   TEST_1,
	   TEST_2
  };
  ```
- Prefer strongly typed enums (enum class instead of enum)
- Prefer nullptr over NULL
- Member variables which are zero or nullptr initialized, can be initialized in header file
  directly instead of using an initializer list in the source file
- Use CamelCase with the first letter upper case for classes, enums and structs
- Use CamelCase with the first letter lower case for class, struct or enum functions
- Try to keep the scope of variables as small as possible
- Write abbreviations in CamelCase too (does not apply to file names)
- For user defined typedefs of objects, use camel case too
  ```cpp
  using std::map<uint32_t, SomeObject> =  ObjectMap
  ```

### Common errors and crash causes in C++/C and basic concepts

In C/C++, the programmer is given a lot of power over how to use the given hardware
without abstraction layers like in other high level languages like Python
or Java. Not knowing how to use this power properly leads to undefined behaviour
in many cases. TLDR: In C/C++, one often gets crashes where the root of the problematic
is difficult to find. Memory allocation is a powerful tool which can also lead to many difficult-to-track
problems at run-time because any allocated memory needs to be freed.
As such, it should be avoided in the Flight Software, unless the size is fixed at compile time / code start-up time.
Here is a list of common errors (please correct if anything is wrong...)

1. Avoid dynamic memory allocation during run-time (e.g. in performOperation() method). the keyword new
allocated dynamically and must be followed by a delete eventually. Try to use static/local variables
where possible and/or initialize arrays or buffers with a maximum size at class instantiation. std libraries
and functions like map and vector use dynamic memory allocation and should be used with care.
However, also keep in mind that most objects have the whole run-time lifetime.
2. Uninitialized variables can lead to undefined behaviour, especially in optimized builds ! It is preferrable
to always initialize variables. It is perfectly possible that code works with uninitialized variables but
some compiler optimizations can lead to undefined behaviour where debug code previously worked.
3. When initializing pointers, be careful with nullptr pointer initializations !
E.g. dereferencing a nullptr pointer leads to a nullptr-Pointer exception (crash 0x4/0x10).
In general, accessing or dereferencing any forbidden memory areas leads to undefined behaviour / crashes.
4. One should get familiar with the concept of pointers and OOP when working with the flight software.
Pointers are uses extensively for buffered data (a buffer is basically an array of bytes).
A pointer is always just an address to a memory location, not an array/list like in other languages like Python !
Therefore, when passing buffered data in C/C++, the size of the data is always needed in addition to the pointer to the start of the buffered data.

Recommended book to learn C++: A Tour of C++ (2nd Edition) by the creator of C++.
