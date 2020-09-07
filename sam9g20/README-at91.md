## <a id="top"></a> <a name="at91"></a> AT91SAM9G20-EK getting started

#### Setting up eclipse to execute the makefile
1. Right click on project sourceobsw-at91sam9g20-ek &rarr; Properties &rarr; C/C++ Build &rarr; use as build command (wsl can be omitted if Windows Build Tools was installed): wsl make -j4. 
![Build command](doc/readme_img/build_command.png)
2. Now software can be built by clicking the hammer symbol
3. Please note that Eclipse CDT has own environmental variables (which are deduced from the native ones normally). If there are some issues running the SDRAM configuration, check whether the used executables are included in the environment variables by going to the project settings (right click project->Properties) to C/C++ Build -> Environment and checking the PATH. The settings will only be applied to the current configuration unless AllConfigurations is selected above. Also make sure that the ARM Toolchain was added to the system environment variables (or add them to the Eclipse environment variables).

#### Loading Software to AT91SAM9G20-EK
1.	Install J-Link ARM software from https://www.segger.com/downloads/jlink/#J-LinkSoftwareAndDocumentationPack (on windows)
2. Check if arm-none-eabi-gdb.exe is found. Otherwise add path to system environement variables. Should already have been installed earlier by
````sh
xpm install --global @gnu-mcu-eclipse/arm-none-eabi-gcc
````
3. Check jumpers on the board. Should be set as follows

![jumpers_at91sam9g20-ek](doc/readme_img/jumpers_at91sam9g20-ek.png)

4.	Connect J-Link to USB port of host computer
5.	Connect J-Link to AT91SAM9G20-EK
6.	Power on AT91SAM9G20-EK
7. Execute make sdramCfg to configure the sdram. Can be done by creating a new target: Right click project &rarr; Build Targets &rarr; change build command to:<br />
wsl make -j4

![build_taget](doc/readme_img/build_target.png)

#### Start J-Link debugging session from Eclipse
1. Right click on project &rarr; Debug As &rarr; Debug Configurations...
2. In the shown menu right click GDB SEGGER J-Link Debugging &rarr; new
3. Insert in field "C/C++ Application" sourceobsw-at91sam9g20-ek-sdram.elf file (located in bin directory)
4. Set up the debugger as shown in the following pictures. Important is that the path to the JLinkGDBServerCL.exe and the arm-none-eabi-gdb.exe are set corretly
5. Now, image can be written to the at91sam9g20-ek by clicking the "Debug"-button
6. Open up Arduino IDE or Puttty with baud rate 115200 to read debug output
Main

![01_jlink_setup](doc/readme_img/01_jlink_setup.png)

Debugger

![02_jlink_setup](doc/readme_img/02_jlink_setup.png)
![03_jlink_setup](doc/readme_img/03_jlink_setup.png)

Startup

![04_jlink_setup](doc/readme_img/04_jlink_setup.png)
![05_jlink_setup](doc/readme_img/05_jlink_setup.png)
<br>

##  Setting up Eclipse environment for build targets
### Example environment

<a name="buildtargets"></a>
Environment uses the previously mentioned build targets. Build targets can be created by
right clicking on sourceobsw -> Build Target -> Create. Also make sure the toolchain is included like
seen in the following picture by going to C/C++ Build -> Settings, checking that the paths for the ARM Toolchain are set correctly and 
hitting Apply.
![Build Target](doc/readme_img/eclipse_example1.PNG)

### Example Build Configuration.
![Example2](doc/readme_img/eclipse_example2.PNG)
![Example3](doc/readme_img/eclipse_example3.PNG)
![Example3](doc/readme_img/eclipse_example4.PNG)
