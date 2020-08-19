<a id="top"></a>
## <a name="stm32"></a>STM32 Nucleo-144 H743ZI Getting Started

1. Install and set up Eclipse with Gnu MCU Eclipse as specified
2. Set up OpenOCD. In Eclipse, go to Window -> Preferences -> MCU and
  set the respective Toolchains for Global Arm Toolchain Path and Global OpenOCD Path. xPacks installation should be recognized
3. Create Debug configuration in eclipse with OpenOCD as tools and
Check whether path to OpenOCD.exe was found, if not: add  manually
4. Add to debug configuration: -f "board/st_nucleo_h743zi.cfg" oder the right configuration for
the used board (can be found in board/ folder of OpenOCD9)
5. OpenOCD configuration GDB settings: Look for arm-none-eabi-gdb.exe in arm-none-eabi-gcc folder (location of xPack should be
seen above) and add full path to GDB settings.
6. Install USB Driver for ST-Link module: [STSW-LINK009](https://www.st.com/content/st_com/en/products/development-tools/software-development-tools/stm32-software-development-tools/stm32-utilities/stsw-link009.html#get-software)
7. Run Debugger with OpenOCD configuration. Board is flashed automatically

There is a possible race condition (error localhost:3333: No connection could be made because the target machine actively refused it.)
If this is the case, add the following line to the GDB Client Setup command in the OpenOCD debug configuration: shell sleep 1


##  Beispiel FSFW auf STM32HZ43ZIT6 Nucleo-144

- verwendet STM32Cube HAL Lib für Hardwareinitialisierung
- Beispiele für FSFW Objekte (siehe init_mission.cpp)
- UART Schnittstelle konfiguriert und print Funktion implementiert, Nachrichten
  können über ein Terminalprogramm gelesen werden (Putty, Eclipse, Arduino IDE).
  Dabei Serial Input auswählen, baud rate = 9600
- Netzwerkschnittstelle konfiguriert und UDP Echo Task implementiert, schickt
  genau das was empfangen wurde wieder zurück,
  siehe UdpReceiveTask.cpp, auf Basis von LwIP und der raw API
- Echoserver wurde unter Windows getestet mit EchoTool und Wireshark.
  Beispiel zur Verwendung von EchoTool: echotool IP_address /p udp /r 7 /l 2008 /n 15 /t 2 /d Testing LwIP UDP echo server.
  Mit Wireshark kann dann die Antwort "Testing LwIP UDP echo server" gelesen werden
- Für Linux basierstes OS mit Netcat (getestet mit Raspbian) senden mit:
```sh
> sudo nc -u IP_Adresse 7
````
d.h. Server Port ist 7
 empfangen mit:
 ```sh
> sudo nc -u -l 2008
````
d.h. Server sendet an Port 2008
- Oder mit Python Skript im python_telecommand Ordner
- Anwendung wartet kurz auf die Zuweisung einer IP Adresse per DHCP.
-Sollte kein DHCP Server vorhanden sein wird automatisch die statische IP Adresse, definiert in main.h, gesetzt.
Die IP Adresse wird über die UART Schnittstelle ausgegeben.
