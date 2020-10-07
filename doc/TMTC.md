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
command the iOBC or AT91 development board, which is the meaning of the -c 1 flag.
QEMU users need to specify -c 2 and Linux users need to specify -c 3 for the 
UDP communication interface.

A timeout for reply checking can be specified by adding the -t \<timeout duration\> flag to the
command for sequential commands (all commands which are specified with -m 3).

## General commands

### Display Help
```sh
python3 obsw_tmtc_client.py -h
``` 
 
### Ping software

A ping command uses the PUS test service (17)
```sh
python3 obsw_tmtc_client.py -m 3 -s 17 -c 1 
```

### Service tests

Perform a service test which should work without connected hardware.
Service tests were implemented for the services 2, 5, 8, 9, 17 and 200.

```sh
python3 obsw_tmtc_client.py -m 3 -s <serviceNumber> -c 1
```

## SD-Card and Image Handling

### General SD-Card commands

Print contents of active SD card
```sh
python3 obsw_tmtc_client.py -m 3 -s SD -o A2 -c 1 
``` 

Clear active SD card
```sh
python3 obsw_tmtc_client.py -m 3 -s SD -o A20 -c 1
``` 

Format active SD card
```sh
python3 obsw_tmtc_client.py -m 3 -s SD -o A21 -c 1
```


