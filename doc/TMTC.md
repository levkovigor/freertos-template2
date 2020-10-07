## Common TMTC commands and procedures

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

### Display Help
```sh
python3 obsw_tmtc_client.py -h
``` 
 
### Ping software

A ping command uses the PUS test service (17)
```sh
python3 obsw_tmtc_client.py -m 3 -s 17 -c 1 
```
