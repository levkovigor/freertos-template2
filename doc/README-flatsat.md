<a id="top"></a> <a name="flatsat"></a> 

The OBC engineering model was set up in the clean room at a flatsat computer which allows remote
development and software deployment.

## Basic instructions for Flatsat

### Setting up the VPN

1. Set up VPN, [IRS  mail account](https://cube18.irs.uni-stuttgart.de/) required: 
After that, write mail to zert@irs.uni-stuttgart.de to get OpenVPN configuration files.

#### Windows

1. Download [OpenVPN](https://openvpn.net/download-open-vpn/) and install it and configure it with 
the configuration files.
2. Put OVPN configuration files into the OpenVPN config folder or add the configuration in the 
OpenVPN GUI
   
#### Ubuntu

1. Install the ubuntu gnome version of OpenVPN
2. Store the VPN files into some folder, for example a VPN folder in the DOocuments folder
3. Go to Settings &rarr; Network &rarr; VPN and press + &rarr; Import From File.
4. Select the .ovpn file previous stored somewhere
5. Now you can activate the VPN via the network button on the top right. Go to the VPN settings to 
the IPv4 settings and set a tick at "Use this connection only resources on its network" to allow
other services like Mattermost.
   

### Connecting to the flatsat computer
   
1. Connect to the VPN
2. Connect to Flatsat (password needed, ask Jonas Burgdorf on Mattermost):

   ```sh
   ssh -X source@flatsat.source.absatvirt.lw
   ```
   
   You can also use the IP address (this is required for PyCharm remote configurations)
   
   ```sh
   ssh -X source@192.168.199.228
   ```
   
3. It is recommended to set up SSH configuration either in Eclipse (cross-platform
and convenient solution) via the terminal feature (terminal button at the top) or
via Putty or Unix Alias. -X is optional for graphical applications.
There is also another command for port forwarding:

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

### Setting up the Flatsat computer for remote deployment

Under normal circumstances, it should not be necessary to change anything on
the flatsat computer directly because all necessary additional tools run in
separate `tmux` terminal sessions. However, these sessions are terminated on restart
and are not set up automatically on restart so it is recommended to checkout
whether all of them are working when starting a debug session. In any case,
it is recommended to connect to the `tmux` session listening to the debug
output fromt the iOBC.

To verify all require services are running, it is recommended to open two `ssh`
sesions in Eclipse and then run following command on the flatsat

```sh
tmux ls
```

This should show following 4 tmux sessions

```sh
source@flatsat:~$ tmux ls
0_iobc: 1 windows (created Thu Nov 19 14:48:17 2020)
1_vor: 1 windows (created Thu Dec 10 12:12:55 2020)
2_iobc_dbg: 1 windows (created Thu Dec 10 12:33:31 2020)
3_vor_dbg: 1 windows (created Thu Dec 10 13:44:08 2020) (attached)
```

Session 1 and 3 are related to the Vorago operations. The remote GDB server required
to perform remote debugging is running inside the session `1_vor` and the debug output
from the Vorago is tracked in the `3_vor_gdb` session.

If the sessions show up, connect to them with the following commands in separate `ssh` sessions

Debug output:
```sh
tmux a -t 2*
```

GDB Server:
```sh
tmux a -t 0*
```

If these sessions don't show up (e.g. after flatsat reboot), here are the 
commands to get everything working again with the following command
<<<<<<< HEAD

```py
setup_flatsat_tmux.py
```

This will run a script located in the `$HOME/scripts` to set up all tmux sessions. 
All scripts used here are located in `$HOME/scripts` as well.

=======

```py
setup_flatsat_tmux.py
```

This will run a script located in the `$HOME/scripts` to set up all tmux sessions. 
All scripts used here are located in `$HOME/scripts` as well.

>>>>>>> mueller/master
Alternatively, all steps can be performed manually.
Initiate the iOBC GDB Server in a new tmux session
```sh
tmux new -s 0_iobc
gdb_iobc.sh
```

`vor_iobc.sh` is a shell script which will run the GDB Server with the correct configuration. 
<<<<<<< HEAD
It runs the following command:

```sh
JLinkGDBServerCLExe -device AT91SAM9G20 -endian little -ir JTAG -speed auto -noLocalhostOnly -select USB=261002202 -nogui
=======
It runs the following command. Please note that the USB S/N number can change depending on J-Link
adapter used:

```sh
JLinkGDBServerCLExe -device AT91SAM9G20 -endian little -ir JTAG -speed auto -noLocalhostOnly -select USB=20127716
 -nogui
>>>>>>> mueller/master
```

Then type `CTRL` + `B` and `d` to detach from the tmux session.
Set up the debug session with the following commands:

```sh
tmux new -s 2_iobc_dbg
listen_iobc.sh
```

You will be prompted for a USB serial port. Select the port named 
`TTL232R-3V3` to listen to the iOBC serial output.

The shell script will start the `screen` utility to read the USB port 
with the correct settings.
Now the debug output can be read in this session.
To exit the session, use `CTRL` + `B` and `d`.

All scripts are located inside the scripts folder in the home folder.

### Building and flashing software on flatsat computer using the flatsat computer

1. Navigate to obsw folder
```sh
obsw
```

2. JLink GDB server needs to run on the flatsat. It is run on standard port 2331
with tmux in the normal case. To check whether a tmux is active, use `tmux ls` .
If a tmux is active, check the status of the GDB server can be checked by using
`tmux a -t 0*`, detaching from the tmux session and moving it to the background
is done by typing in `CRTL+B`, `:` and `d`. To close the tmux session,
use `k` instead of `d` instead. In some cases, it can becomes necessary to restart
the J-Link GDB Server. The GDB Server should be run with the following command

```sh
JLinkGDBServerCLExe -device AT91SAM9G20 -endian little -ir JTAG -speed auto -noLocalhostOnly -select USB=20127716
 -nogui
```

Background processes can be listed with `ps -aux` and killed with `kill <processId>`

3. Binary can be built locally with
```sh
make IOBC=1 virtual -j2
```
Or mission instead of virtual for mission build.

4. Perform sdramCfg (only needs to be once after power cycle)
```sh
make sdramCfg
```

5. Connect to the tmux session which is listening to the USB port. 
You can check whether the tmux exists with the command `tmux ls`.
Follow the steps specified in the previous section if it does not exist
to create a new tmux serial listener session.

```sh
tmux a -t 2_*
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

### Loading binaries built locally to the non-volatile memory

It is recommended to flash the software to the SDRAM directly for development purposes. 
To test the binary and the bootloader on  the non-volatile memories, the images need to be written
to the 1MB NOR-Flash chip. This is either possible with SAM-BA when interfacing the iOBC with a 
Windows PC and the ISIS SAM-BA application installed or by uploading the binary via RS232 
(same communication line used for TMTC commanding). For remote deployment, only the second
way is currently possible. A recent software version needs to 
be running to perform this step as well. Following general steps need to be taken:

1. Transfer the file with to the \_bin folder of
the remote OBSW folder with SFTP. It is recommended to use FileZilla for this.
It is possible to set common operations as favorites in Filezilla.

2. Transfer the binary to the SD-Card first. The `tmtcclient` Python application
inside the `tmtc` folder can be used to either transfer an OBSW Update or a bootloader.
This mode is provided as a PyCharm run configuration when loading
the `tmtc` folder as a PyCharm project.

3. After that, a specific command provided by the `tmtmcclient` can be used 
to write the  bootloader or OBSW image from SD-card to the NOR-Flash.

4. Another command can be used to power cycle or reset the core to test the flashed
software


### Preparing the Eclipse without provided run configurations

It is recommended to use the supplied launch configurations and project files
instead of rerunning these steps.

1. The current IP address of the flatsat computer is 
<<<<<<< HEAD
192.128.199.228 . That address could change, and it can be checked
by logging into the flatsat like explained above and running: 
 
```sh
ifconfig    
```
=======
   192.168.199.228 . That address could change, and it can be checked
   by logging into the flatsat like explained above and running: 
 
   ```sh
   ifconfig
   ```
>>>>>>> mueller/master
  
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


