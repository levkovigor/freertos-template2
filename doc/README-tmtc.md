# Common TMTC commands and procedures

The TMTC commander is integrated into the repository as a submodule.
It provides an easy way to developers and operators to test the software
with telecommands and read the telemetry generated.

Common TMTC commands and procedures will be listed here. 
While most important commands are included as PyCharm run configurations,
the listing will help when changing to different operations software
or using another Python IDE.

It is assumed that Python 3.8 is installed. The required steps to make
the program work are shown in the [TMTC readme](https://git.ksat-stuttgart.de/source/tmtc)

Please note that for the commands shown here a serial interface is assumed to
command the iOBC or AT91 development board, which is the meaning of the `-c 1` flag.
QEMU users need to specify `-c 2` and Linux/Host users need to specify `-c 3` for the 
UDP communication interface.

A timeout for reply checking can be specified by adding the `-t <timeout duration>` flag to the
command for sequential commands (all commands which are specified with -m 3).

In the following sections, only the command line arguments will be supplied which you need to run
supply to the `tmtc_client_cli.py` call.

The service argument supplied with `-s <ServiceNumberString>` can be a number or a string.
For numbers, this will generally be a PUS service while for strings, this will be a custom class 
(e.g. `-s img` for the Software Image Handler).

The operation code argument is a string used to define multiple functional tasks for a 
specific service. There are three common prefixes for operation codes (op-codes):

- An op-code starting with `a` will generally be a PUS service 8 command for a specific
Action ID
- An op-code starting with `c` will pack command stacks with multiple commands
- An op-code starting with `p` will pack PUS service 20 commands to change parameters.

## General commands

### Display Help

For the first example, the prefix will be shown in addition to the arguments (in this case, only
`-h` is supplied)

```sh
python3 tmtc_client_cli.py -h
``` 

You can also just run the `tmtc_client_cli.py` file directly.
 
### Ping software

A ping command uses the PUS test service (17)
```sh
-m 3 -s 17 -c 1 
```

Enable periodic printout, using PUS test service 17
```sh
-m 3 -s 17 -o 129 -c 1
```

Disable periodic printout

```sh
-m 3 -s 17 -o 130 -c 1
```

### Core Management

These functions can be used to command and interface the core of the OBC
or its supervisor.

Software reset or supervisor reset
```sh
-m 3 -s Core -o a10 -c 1
```

Supervisor power cycle
```sh
-m 3 -s Core -o a11 -c 1
```

Print run time stats

```sh
-m 3 -s Core -o a0 -c 1
```

Trigger a software exception which should lead to a restart

```sh
-m 3 -s 17 -o 150 -c 1
```

### Service tests

Perform a service test which should work without connected hardware.
Service tests were implemented for the services 2, 5, 8, 9, 17 and 200.

```sh
-m 3 -s <serviceNumber> -c 1
```

## SD-Card and Image Handling

### General SD-Card commands

Print contents of active SD card

```sh
-m 3 -s sd -o a2 -c 1 
``` 

Clear active SD card
```sh
-m 3 -s sd -o a20 -c 1
``` 

Format active SD card

```sh
-m 3 -s sd -o a21 -c 1
```

Generate generic folder structure, `-o c0a` for AT91, `-o c0i` for iOBC

```sh
-m 3 -s sd -o c0a -c 1
```

Lock file on SD card. Locked files are read-only and can not be deleted.
The all directories containing a locked file can not be deleted as well.
```sh
-m 3 -s sd -o 5 -c 1
```

Unlock file on SD card

```sh
-m 3 -s sd -o 6 -c 1
```

###  Software Update Procedure

Upload the bootloader or the software image using the special
binary upload mode. For the command line it is assumed that the software was built previously, 
using the instructions in the 
[main REAMDE](https://git.ksat-stuttgart.de/source/sourceobsw/-/blob/master/README.md) and is 
located in the \_bin folder (the software will only look for bin files in that folder!).
For the GUI mode, the binary can be located anywhere.

Upload bootloader(s) or software image

```sh
-m 5 -c 1
```

Copy OBSW image to boot memory

```sh
-m 3 -s img -o a3u -c 1
```

Copy bootloader image to boot memory. Use `a16a1` to copy the second-stage bootloader for the AT91.

```sh
-m 3 -s img -o a16a0 -c 1 
```

Test whether binary was uploaded successfully: 
Power cycle the OBC, either externally or via following commands
shown above for core management.
