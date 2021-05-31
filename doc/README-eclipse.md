#  Setting up Eclipse environment for build targets

## General information

Eclipse is a major IDE used for various programming languages. While it might
be better known as an IDE for Java development, it is possible to develop
C and C++ code (for Desktop or for microcontrollers) with the right plugins and 
extensions. Eclipse is a very useful tool to ease development and provides 
tools like a powerful project indexer and various ways to conveniently build, 
run and debug software on either the desktop or on embedded systems connected 
via J-Link or USB.

However, it is also somewhat cumbersome and difficult to set up for beginners.
Therefore, some recommended starting points will be given to set up Eclipse
properly for convenient development.

## Installation Eclipse for C/C++ Developers on Windows

1. Install [Eclipse for C/C++ Developers](https://www.eclipse.org/downloads/packages/) using
   the installer
2. Go to Menu &rarr; Help &rarr; Eclipse Marketplace and search and install GNU MCU Eclipse


## Recommended steps

The recommended steps show how to build `CMake` projects with Eclipse. The `CMake` build
system generation still has to be done separately via the `CMake` GUI or via command line
with the provided configuration scripts.

### Pre-Configured project

Perform following steps to use a pre-configured project.

1. Copy the  files `.project` and `.cproject` inside the misc/eclipse folder 
   into the root of the cloned folder.
2. Import the project by going to File &rarr; Import &rarr; Existing Projects into Workspace and 
   selecting (only!) the cloned folder.
  
## Additional Information

### Cross compiler path

It is not recommended anymore to add the cross-compile toolchain to the system environmental
variables. Eclipse still requires access to utilities like `arm-none-eabi-gdb`. 
Because a xPacks cross compiler toolchain is used, the toolchain path should be added to the
Eclipse internal path variables automatically as long as the MCU settings for Eclipse 
were configured properly.

### Lauch Configurations

Launch configuration were provided. These configurations use the build configuration provided
by the `.cproject` file and perform steps like starting the `JLink` software or the 
debugger.

