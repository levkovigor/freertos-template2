# <a id="top"></a> <a name="linux"></a> Linux
Please note that a full linux installation should be available,
either inside a virtual machine or as dualboot or stand-alone installation to run
the linux binary or the unit test.<br>
These steps were tested for Ubuntu 18.04 and Ubuntu 19.10. 
Steps might vary for other distributions.
If not done yet, install the full C++ build chain:
```sh
sudo apt-get install build-essential
```

After that, the linux binary can be built with:
```sh
make -f Makefile-Linux
```
to compile for Linux.
Please note that on most UNIX environments (e.g. Ubuntu), the real time functionalities 
used by the UNIX pthread module are restricted, which will lead to permission errors when creating these tasks.

To solve this issues, try following steps:

1. Run the shell script inside the linux folder
```sh
./unlockRealtime
```
This script executes the `setcap` command on bash and on the binaries.
It also increases the soft real time limit of the current shell instance
to the maximum and increases the maximum number of message queues.
All changes are only applied for the current session (read 2. and 3. for 
a permanent solution).
If running the script before executing the binary does
not help or an warning is issue that the soft real time value is invalid, 
the hard rteal time limit of the system might not be high enough.
Try the following step.

2. Edit the /etc/security/limits.conf 
file and add following lines at the end:
```sh
<username>   hard   rtprio  99
<username>   soft   rtprio  99
```
The soft limit can also be set in the console with `ulimit -Sr` if the hard
limit has been increased, but it is recommended to add it to the file as well for convenience.
If adding the second line is not desired for security reasons,
the soft limit needs to be set for each session. If using an IDE like eclipse 
in that case, the IDE needs to be started from the console after setting
the soft limit higher there. After adding the two lines to the file,
the computer needs to be restarted (it is possible that relogging is enough)

It is also recommended to perform the following change so that the unlockRealtime
script does not need to be run anymore each time. The following steps
raise the maximum allowed message queue length to a higher number permanently, which is 
required for some framework components. The recommended values for the new message
length is 120.

3. Edit the /etc/sysctl.conf file
```sh
sudo nano /etc/sysctl.conf
```
Append at end: 
```sh
fs/mqueue/msg_max = <newMsgMaxLen>
```
Apply changes with: 
```sh
sudo sysctl -p
``` 

A possible solution which only persists for the current session is
```sh
echo <newMsgMax> | sudo tee /proc/sys/fs/mqueue/msg_max
```
or running the `unlockRealtime` script.

The Linux binary has an own main (see linux folder), does not do much and still 
needs to be developed.


## <a name="unittest"></a> Unit Tests
Refer to the [egit repostiory readme](https://egit.irs.uni-stuttgart.de/fsfw/fsfw_tests).<br>
Leave out step 1, and run make with following command:
```sh
make -f  Makefile-Unittest all
```

Furthermore, the unlockRealtime script is located in the linux folder now.
Right now, the Unittest does not make use of pthread yet, but consider the
steps above to unlock real time functionalities if they are used.
