# PUS Tests
 
All tests can be found in the code
[here](https://git.ksat-stuttgart.de/source/tmtc/-/tree/development/pus_tc)

## PUS Service 8

Generic tests which works without hardware.
Only works if test device is enabled:

1. Send TC[200,1] to command mode On
2. Send TC[200,1] to command mode Normal
3. Send TC[8,128] to trigger completion reply
4. Send TC[8,128] to trigger data reply
5. Send TC[8,128] to trigger step and completion reply
6. Send TC[200,1] to command mode Off

## PUS Service 2

Generic tests which works without hardware.
Only works if test device is enabled:

1. Send TC[200,1] to command mode Raw
2. Send TC[2,129] to toggle wiretapping Raw
3. Send TC[2,128] to send raw command
4. Send TC[2,129] to toggle wiretapping Off
5. Send TC[2,128] to send second raw command
6. Send TC[200,1] to command mode Off

## PUS Service 3

1.

## PUS Service 17

1. Ping with TC(17,1) and verify TC verification TM(1,1), TM(1,3) and TM(1,7)

## PUS Service 200

Generic tests which works without hardware.
Only works if test device is enabled:

1. Send TC[200,1] to command mode On
2. Send TC[200,1] to command mode Normal
3. Send TC[200,1] to command mode Raw
4. Send TC[200,1] to command mode Off
