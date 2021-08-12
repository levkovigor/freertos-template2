<a id="top"></a> <a name="flatsat"></a> SOURCE Flatsat
======

The OBC engineering model was set up in the clean room at a flatsat computer which allows remote
development and software deployment.

# Basic instructions for Flatsat

## Setting up the VPN

1. Set up VPN, [IRS  mail account](https://cube18.irs.uni-stuttgart.de/) required: 
After that, write mail to zert@irs.uni-stuttgart.de to get OpenVPN configuration files.

### Windows

1. Download [OpenVPN](https://openvpn.net/download-open-vpn/) and install it and configure it with 
the configuration files.
2. Put OVPN configuration files into the OpenVPN config folder or add the configuration in the 
OpenVPN GUI
   
### Ubuntu

1. Install the ubuntu gnome version of OpenVPN
2. Store the VPN files into some folder, for example a VPN folder in the DOocuments folder
3. Go to Settings &rarr; Network &rarr; VPN and press + &rarr; Import From File.
4. Select the .ovpn file previous stored somewhere
5. Now you can activate the VPN via the network button on the top right. Go to the VPN settings to 
   the IPv4 settings and set a tick at "Use this connection only resources on its network" to allow
   other services like Mattermost.
   

# Connecting to the flatsat computer
   
1. Connect to the VPN
2. Connect to the flatsat (password needed, ask Jonas Burgdorf on Mattermost):

   ```sh
   ssh -X source@flatsat.source.absatvirt.lw
   ```
   
   You can also use the IPv6 address:

   ```sh
   ssh -X source@2001:7c0:2018:1099:babe:0:50ce:f1a5
   ```

   or the IPv4 but this one is not static
   
   ```sh
   ssh -X source@192.168.199.178
   ```
   
3. It is strongly recommended to set up a `ssh` tunnel for TMTC commanding. The port forwarding
   will cause requests sent to localhost port 2336 on the development host to be sent to the port
   2331 of the flatsat, which is used by the J-Link GDB Server application. It will also cause
   TMTC requests sent to port 2336 of localhost or address 127.0.0.1 to be forwarded to port
   7301 of the flatsat computer. As long as the `tmtc-agent` utility is running on the flatsat
   computer, you can command run the `tmtc` application on your host machine with the TCP
   communication interface. You can set up the port forwarding by running

   ```sh
   ./scripts/source-port.sh
   ```

   Or run the full command:

   ```sh
   ssh -L 2336:localhost:2331 \
       -L 2337:localhost:7301 \
       source@flatsat.source.absatvirt.lw -t \
       'CONSOLE_PREFIX="[SOURCE Port] /bin/bash'
   ```

4. Check whether all required `tmux` sessions are running on the flatsat.
   If not, run `setup-flatsat-tmux.py`. See the [setup](#gdb-debug) chapter for more details.

5. Check that the `tmtc-agent` systemd service is running by running
   `systemctl status tmtc-agent`. See the [TMTC Agent](#tmtc-agent) chapter for more details.

# Common commands
 
Build software for debugging:

```sh
mkdir build-Mission-iOBC && cd build-Mission-iOBC
cmake -DBOARD_IOBC=ON ..
cmake --build . -j
```
Build release software:

```sh
mkdir build-Mission-iOBC && cd build-Mission-iOBC
cmake -DBOARD_IOBC=ON -DCMAKE_BUILD_TYPE=Release ..
cmake --build . -j
```

Build bootloader:

```sh
mkdir build-Mission-BL-iOBC && cd build-Mission-BL-iOBC 
cmake -DBOOTLOADER=ON -DBOARD_IOBC=ON -DAT91_NO_FREERTOS_STARTER_FILE=ON -DCMAKE_BUILD_TYPE=Release .. 
cmake --build . -j
```

# <a id="setup"></a> Setting up the Flatsat computer for Remote Deployment

## <a id="tmtc-agent"></a> TMTC Agent for convenient TMTC commanding

The `tmtc-agent` systemd service will start the TCPIP to Serial Agent for the iOBC. The application
can be found in the in the `scripts` folder.

The configuration of this application can be found at the
`KSat/tcpip-to-serial-agent/agent_conf.json` file. The most important configuration pairs
are the `serial_port` or the `serial_hint` configuration option. 

It is recommended to add `"serial_hint" : "EVAL232 Board"` to this JSON. This will cause the agent
to forward TCP requests via the correct serial port.

You can find the source code for the application
[here](https://git.ksat-stuttgart.de/source/tcpip-to-serial-agent)

## <a id="gdb-debug"></a> GDB Server and Debug Terminal

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

```py
setup-flatsat-tmux.py
```

This will run a script located in the `$HOME/scripts` to set up all tmux sessions. 
All scripts used here are located in `$HOME/scripts` as well.

Alternatively, all steps can be performed manually.
Initiate the iOBC GDB Server in a new tmux session
```sh
tmux new -s 0_iobc
gdb-iobc.sh
```

`gdb-iobc.sh` is a shell script which will run the GDB Server with the correct configuration.
It runs the following command. Please note that the USB S/N number can change depending on J-Link
adapter used:

```sh
JLinkGDBServerCLExe -device AT91SAM9G20 -endian little -ir JTAG -speed auto -noLocalhostOnly \
    -select USB=20127716 -nogui
```

Then type `CTRL` + `B` and `d` to detach from the tmux session.
Set up the debug session with the following commands:

```sh
tmux new -s 2_iobc_dbg
listen-iobc.sh <devPort>
```

Specify the port named  `TTL232R-3V3` to listen to the iOBC serial output.

The shell script will start the `picocom` utility to read the USB port 
with the correct settings.
Now the debug output can be read in this session.
To exit the session, use `CTRL` + `B` and `d`.

All scripts are located inside the scripts folder in the home folder.

# Building and flashing software on flatsat computer using the flatsat computer

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
JLinkGDBServerCLExe -device AT91SAM9G20 -endian little -ir JTAG -speed auto \
   -noLocalhostOnly -select USB=20127716 -nogui
```

Background processes can be listed with `ps -aux` and killed with `kill <processId>`

3. Binary can be built locally with the `CMake` command shown above.

4. Perform sdramCfg (only needs to be once after power cycle)
   ```sh
   ./sdram-cfg.sh
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
   load <build_folder>/<file_name>.elf
   ```
and press c to start

# Loading binaries built locally to the non-volatile memory

It is recommended to flash the software to the SDRAM directly for development purposes. 
To test the binary and the bootloader on  the non-volatile memories, the images need to be written
to the 1MB NOR-Flash chip. This is either possible with SAM-BA when interfacing the iOBC with a 
Windows PC and the ISIS SAM-BA application installed or by uploading the binary via RS232 
(same communication line used for TMTC commanding). For remote deployment, only the second
way is currently possible. A recent software version needs to 
be running to perform this step as well. Following general steps need to be taken:

1. Transfer the file with to the `sourceobsw` folder of the remote OBSW folder with SFTP. 
   It is recommended to use FileZilla for this. It is possible to set common operations as
   favorites in Filezilla.

2. Transfer the binary to the SD-Card first. The TMTC Python application
   inside the `tmtc` folder can be used to either transfer an OBSW Update or a bootloader.
   This mode is provided as a PyCharm run configuration when loading
   the `tmtc` folder as a PyCharm project.

3. After that, a specific command provided by the TMTC application can be used 
   to write the  bootloader or OBSW image from SD-card to the NOR-Flash.

4. Another command can be used to power cycle or reset the core to test the flashed
   software
