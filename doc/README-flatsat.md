<a id="top"></a> <a name="flatsat"></a> 

The OBC engineering model was set up in the clean room at a flatsat computer which allows remot development and software deployment.

## Basic instructions for Flatsat

### Connecting to the flatsat computer

1. Set up VPN, [IRS  mail account](https://cube18.irs.uni-stuttgart.de/) required: 
Write mail to zert@irs.uni-stuttgart.de to get OpenVPN configuration.
2. Download OpenVPN and configure it with the configuratioon files.
 - Windows: Put configuration files into the OpenVPN config folder
   or add the configuration in the OpenVPN GUI
 - Ubuntu: Install the ubuntu gnome version of OpenVPN. Then go to 
   Network &rarr VPN and press + to add the .ovpn file configuration
3. Connect to the VPN
4. Connect to Flatsat (password needed, ask Jonas Burgdorf on Mattermost):
   ```sh
   ssh (-X) source@flatsat.source.absatvirt.lw
   ```
   
   It is recommended to set up SSH configuration either in Eclipse (cross-platform
   and convenient solution) via the terminal feature (terminal button at the top) or
   via Putty or Unix Alias.
   -X is optional for graphical applications
   There is also another command for port forwarding
   ```sh
   ssh -L <localPort>:localhost:<remotePort> source@flatsat.source.absatvirt.lw
   ```
   to tunnel from <localPort> to the <remotePort> on the flatsat.

### Common commands
When building on the flatsat computer directly, it is recommended to add
ADD\_CR=1 so that debug output is readable.
 
Build software for debugging
```sh
make debug IOBC=1 -j
```

Build release software
```sh
make mission IOBC=1 -j
```

Build bootloader
```sh
make mission -f Makefile-Bootloader IOBC=1 -j
```

### Building software on flatsat computer
1. Navigate to obsw folder
```sh
obsw
```

2. JLink GDB server needs to run on the flatsat. It is run on standard port 2331
with tmux in the normal case. To check whether a tmux is active, use `tmux ls` .
If a tmux is active, check the status of the GDB server can be checked by using
`tmux a`, detaching from the tmux session and moving it to the background
is done by typing in `CRTL+D`, `:` and `detach`. To close the tmux session,
use `kill-session` instead. In some cases, it can becomes necessary to restart
the J-Link GDB Server. The GDB Server should be run with the following command

```sh
JLinkGDBServerCLExe -select USB=261002202 -device AT91SAM9G20 -endian little -if JTAG -speed auto -noLocalhostOnly -nogui
```
Add a & at the end optionally to run it in the background. Background processes can be listed
with `ps -aux` and killed with `kill <processId>`

3. Binary can be built locally with
```sh
make IOBC=1 virtual -j2
```
Or mission instead of virtual for mission build.

4. Perform sdramCfg (only needs to be once after power cycle)
```sh
make sdramCfg
```

5. Open second shell session, connect to flatsat and run 
```sh
listen_iobc.sh
```
This will only work if the dev path of the debug output
is /dev/ttyUSB0, which will usually be the case and if the baudrate is
115200.
If it is not the case, the connected USB devices can be checked
with `listUsb`and a generic version can be used.
```sh
listen_usb.sh <devPath> <baudRate>
```

6. Start GDB (the following steps can propably be automated, but I don't know how yet.)

```sh
arm-none-eabi-gdb
```

7. Set target in 

```sh
target remote localhost:2331
```

8. Load .elf file

```sh
load _bin/iobc/<folder>/<binarary>.elf
```
and press c to start

### Setting up Eclipse for remote development

1. The current IP address of the flatsat computer is 
192.128.199.228 . That address could change, and it can be checked
by logging into the flatsat like explained above and running:   
```sh
ifconfig    
```
  
It is also assumed the the JLinkGDBServerCLExe application
is already running on the interface computer (either directly
in a shell instance, in the background, or in a tmux)

2. The IP address is used to set up Eclipse for remote development.
A set-up build system for the on-board software in Eclipse is a
pre-requisite and is explained in [AT91SAM9G20 getting started](../sam9g20/README-at91.md#top)
inside the build target section. If the launch configuration has been set-up, it is simply copied by
clicking on "Duplicate" inside the launch configuration settings.

3. Setup a new build configuration. The only different to the AT91 builds
is the added `IOBC=1` make flag. Example make command for mission build
on Windows
```sh
make WINDOWS=1 IOBC=1 mission -j
```
Don't forget to select the binary after building it initially

4. The debugger tab should be set up like show below. The IP address
is the IP address of the iOBC interface computer and can be retrieved
like explained in point 1. 
<img src="./readme_img/flatsat/eclipse-setup1.jpg" width="50%">

5. The startup tab should be set up like below. Right now, the 
SAM-ICE can not handle monitor reset or monitor halt commands
and the reason is unknown.
<img src="./readme_img/flatsat/eclipse-setup2.jpg" width="50%">

6.  The serial output from the iOBC can be read from the dev path of the
interface computer directly. It is possible in Eclipse to open a ssh
session like shown in the following picture.
<img src="./readme_img/flatsat/eclipse-setup3.jpg" width="50%">
It is recommended to listen to the debug output by connecting
to the tmux session with

```sh
tmux a -t 2*
```

Alternatively ways if the tmux session is closed:

The output can be display by running these commands
in the ssh session:
```sh
listen_iobc.sh
```

There is also a generic version to listen to USB ports:
```sh
listen_usb.sh <DevPath> <baudRate>
```

All dev paths can be listed with the command

```sh
list_usb.sh
```

or 

```sh
list_usb2.sh
```

These scripts are located inside the scripts folder in the home folder.

### Loading binaries built locally to the non-volatile memory

It is recommended to flash the software to the SDRAM directly for
development purposes. To test the binary and the bootloader on 
the non-volatile memories, the images need to be written
to the 1MB NOR-Flash chip. This is either possible with SAM-BA
when interfacing the iOBC with a Windows PC and the ISIS SAM-BA application
installed or by uploading the binary via RS232 (same communication line
used for TMTC commanding). For remote deployment, only the second
way is currently possible. A recent software version needs to
be running to perform this step as well.
Following general steps need to be taken:

1. Transfer the file with to the \_bin folder of
the remote OBSW folder with SFTP. It is recommended to use Filezilla for this.
It is possible to set common operations as favorites in Filezilla.

2. Transfer the binary to the SD-Card first. The `tmtcclient` Python application
inside the `tmtc` folder can be used to either transfer an OBSW Update or a bootloader.
This mode is provided as a PyCharm run configuration when loading
the `tmtc` folder as a PyCharm project.

3. After that, a specific command provided by the `tmtmcclient` can be used 
to write the  bootloader or OBSW image from SD-card to the NOR-Flash.

4. Another command can be used to power cycle or reset the core to test the flashed
software

