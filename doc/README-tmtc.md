# Common TMTC commands and procedures

The TMTC commander is integrated into the repository as a submodule.
It provides an easy way to developers and operators to test the software
with telecommands and read the telemetry generated.

Common TMTC commands and procedures will be listed here. 
While most important commands are included as PyCharm run configurations,
the listing will help when changing to different operations software
or using another Python IDE. The current serial baudrate to send 
and receive telecommands is 230400.

It is assumed that Python 3.8 is installed. The required steps to make
the program work are shown in the [TMTC readme](https://git.ksat-stuttgart.de/source/tmtc)

In the following sections, only the command line arguments will be supplied which you need to run
supply to the `tmtc_client_cli.py` call.

The service argument supplied with `-s <ServiceNumberString>` can be a number or a string.
For numbers, this will generally be a PUS service while for strings, this will be a custom class 
(e.g. `-s img` for the Software Image Handler).

You can replace `-c ser_dle` with `-c udp` for the hosted software build.

The operation code argument is a string used to define multiple functional tasks for a 
specific service. There are three common prefixes for operation codes (op-codes):

- An op-code starting with `a` will generally be a PUS service 8 command for a specific
Action ID
- An op-code starting with `c` will pack command stacks with multiple commands
- An op-code starting with `p` will pack PUS service 20 commands to change parameters.

## Execute Before Flight Sequence

Similar to a remove-before-flight pin for hardware, there is a sequence of commands and steps
that should be performed on the integrated satellite before flight. These usually
only need to be set once.

### FRAM 

1. Set seconds since epoch to current time
2. Set reboot counter to 0
3. Enable global hamming code check flag
4. Enable indiviual hamming check flags
5. Set preferred SD card to 0
6. Zero out variables explicitely:
    - All indiviual reboot counters
	- Bootloader faulty flag
	- Software Update available flag
7. Transfer hamming codes to the FRAM from the SD-Card.
   - Hamming code for NOR-Flash binary
   - Hamming codes for SD-Card 0 and SD-Card 1
   - Hamming code for bootloader
8. Transfer the bootloader to the FRAM
9. Dump the critical block and verify everything is set correctly. Check whether hamming codes
   are set by checking size fields

### SD-Card

1. Each SD-Card should have the generic file structure which can be set with a telecommand stack.
   The generic file structure looks like this (already includes images and hamming code files)
   
   ```sh
   INFO: | 21:51:15.033 | Printing SD Card: 
   INFO: | 21:51:15.038 | F = File, D = Directory, - = Subdir Depth
   INFO: | 21:51:15.045 | D: TC
   INFO: | 21:51:15.052 | -D: LARGE
   INFO: | 21:51:15.064 | -D: SMALL
   INFO: | 21:51:15.079 | D: TM
   INFO: | 21:51:15.085 | -D: HK
   INFO: | 21:51:15.097 | -D: SC
   INFO: | 21:51:15.106 | --D: LARGE
   INFO: | 21:51:15.120 | --D: SMALL
   INFO: | 21:51:15.131 | D: BIN
   INFO: | 21:51:15.136 | -D: IOBC
   INFO: | 21:51:15.143 | --D: BL
   INFO: | 21:51:15.150 | ---F: BL.BIN
   INFO: | 21:51:15.155 | ---F: BL_HAM.BIN
   INFO: | 21:51:15.163 | --D: OBSW
   INFO: | 21:51:15.170 | ---F: OBSW_SL1.BIN
   INFO: | 21:51:15.177 | ---F: SL1_HAM.BIN
   INFO: | 21:51:15.177 | ---F: OBSW_SL0.BIN
   INFO: | 21:51:15.177 | ---F: SL10_HAM.BIN
   INFO: | 21:51:15.187 | D: MISC
   ```

2. Each SD-Card should have two images and one bootloader and corresponding hamming codes.

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
-m seqcmd -s 17 -c ser_dle
```

Enable periodic printout, using PUS test service 17
```sh
-m seqcmd -s 17 -o 129 -c ser_dle
```

Disable periodic printout

```sh
-m seqcmd -s 17 -o 130 -c ser_dle
```

### Core Management

These functions can be used to command and interface the core of the OBC
or its supervisor.

Software reset or supervisor reset
```sh
-m seqcmd -s Core -o a10 -c ser_dle
```

Supervisor power cycle
```sh
-m seqcmd -s Core -o a11 -c ser_dle
```

Print run time stats

```sh
-m seqcmd -s Core -o a0 -c ser_dle
```

Trigger a software exception which should lead to a restart

```sh
-m seqcmd -s 17 -o 150 -c ser_dle
```

### Service tests

Perform a service test which should work without connected hardware.
Service tests were implemented for the services 2, 5, 8, 9, 17 and 200.

```sh
-m seqcmd -s <serviceNumber> -c ser_dle
```

## SD-Card and Image Handling

### General SD-Card commands

Print contents of active SD card

```sh
-m seqcmd -s sd -o a2 -c ser_dle
``` 

Clear active SD card
```sh
-m seqcmd -s sd -o a20 -c ser_dle
``` 

Format active SD card

```sh
-m seqcmd -s sd -o a21 -c ser_dle
```

Generate generic folder structure, `-o c0a` for AT91, `-o c0i` for iOBC

```sh
-m seqcmd -s sd -o c0a -c ser_dle
```

Lock file on SD card. Locked files are read-only and can not be deleted.
The all directories containing a locked file can not be deleted as well.
```sh
-m seqcmd -s sd -o 5 -c ser_dle
```

Unlock file on SD card

```sh
-m seqcmd -s sd -o 6 -c ser_dle
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
-m 5 -c ser_dle
```

Copy OBSW image to boot memory

```sh
-m seqcmd -s img -o a3u -c ser_dle
```

Copy bootloader image to boot memory. Use `a16a1` to copy the second-stage bootloader for the AT91.

```sh
-m seqcmd -s img -o a16a0 -c ser_dle
```

Test whether binary was uploaded successfully: 
Power cycle the OBC, either externally or via following commands
shown above for core management.
