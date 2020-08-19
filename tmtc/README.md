TMTC Client
====

## Prerequisites
Runs with Python 3.8.
Don't use Python 2.x!
Manual installation of crcmod and pyserial might be needed.
It is recommended to install use PyCharm to run this client.
Students of the Uni Stuttgart usually have access to PyCharm Professional.
1. Install pip if it is not installed yet
2. Install crcmod and all other required packages if using system python
   compiler. If using the local venv as the compiler, these packages
   should be installed. <br>
   Command: 
   ```sh
   python<version> -m pip<version> install crcmod
   ``` 
   or use IDE (interpreter settings -> pip in PyCharm)
3. Install python-tk on Linux. 
    ```sh
    sudo apt-get install python-tk
    ```
   On Windows, the package should be included.

## How To Use
The script can be used by specifying command line parameters.
Please run this script with the -h flag
or without any command line parameters to display options. 

GUI is work-in-progress.
It might be necessary to set board or PC IP address if using ethernet communication.
Default values should work normally though.

## Examples
Example command to test service 17,
assuming no set client IP (set manually to PC IP Address if necessary) and default board IP 169.254.1.38:
```shell script
obsw_tmtc_client.py -m 3 -s 17
```
Example to run Unit Test:
```shell script
obsw_tmtc_client.py -m 5
```
Example to test service 3 with serial communication, printing all housekeeping packets,
COM port needs to be typed in manually (or set with --COM \<COM PORT>):
```shell script
obsw_tmtc_client.py -m 3 -s 3 --hk -c 
```

## Modes
There are four different Modes:
1. GUI Mode (-m 0): Experimental mode, also called if no input parameter are specified
2. Listener Mode (-m 1): Only Listen for incoming TM packets
3. SingleCommandMode (-m 2): Send Single Command repeatedly until answer is received,
only listen after that. Single command needs to be packed (see function below main())
4. ServiceTestMode (-m 3): Send all Telecommands belonging to a certain service
and scan for replies for each telecommand. Listen after that
5. SoftwareTestMode (-m 4): Send all services and perform reply scanning like mode 3.
Listen after that
6. Unit Test Mode (-m 5): Performs a unit test which returns a simple OK or NOT OK. This mode
has the capability to send TCs in bursts, where applicable

In sequential mode, the Client listens for a specified time before sending the next
telecommand. This time has a default value but can be set with the -t parameter (e.g. -t 12).
The TC timeout factor is mulitplied with the TM timeout to specifiy
when a TC is sent again, if no reply is received. 

## Ethernet Communication
If there are problems receiving packets, use the tool Wireshark to track ethernet communication
for UDP echo packets (requests and response).
If the packets appear, there might be a problematic firewall setting.
Please ensure that python.exe UDP packets are not blocked in advanced firewall settings
and create a rule to allow packets from port 2008.

## Serial Communication
Serial communication was implemented and is tested for Windwos 10.
It requires the PySerial package installed.

## Module Test
Includes a moduel tester which sends TCs in a queue and automatically
analyzes the replies. This is the best way to test the functionality of the 
software right now as a software internal TC injector has not been implemented
yet for the FSFW.
Some more information will follow on how to write Unit Tests.

## Developers Information
Code Style: [PEP8](https://www.python.org/dev/peps/pep-0008/).

Can be enforced/checked by using Pylint as an external program in PyCharm.
Install it with pip and then install and set-up the Pylint plugin in PyCharm.

There are a lot of features which would be nice, for example a GUI.
The architecture of the program should allow extension like that without
too many issues, as the sending and telemetry listening are decoupled.

## Import run configurations in PyCharm
The PyCharm IDE can be used to comfortably manage a set of run configuations (for example tests for different
services). These configurations were shared through the version control system Git
 and should be imported automatically. If these configurations dont show up, try to open the tmtc folder as
 a new PyCharm project in a new window. 
 
 To add new configurations, go to Edit Configurations... at the top right corner in the drop-down menu.
 Specify the new run configurations and set a tick at Share through VCS. 