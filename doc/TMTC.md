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

Please note that it is always assumed a serial interface is used to
command the iOBC or AT91 development board (-c 1).
The timeout can be specified by adding the -t \<timeout duration\> flag to the
command
 
### Ping software

A ping command uses the PUS test service (17)
```sh
python3 obsw_tmtc_client.py -m 3 -s 17 -c 1 
```
