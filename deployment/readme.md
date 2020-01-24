# Model Deployment

[![Line Following Robot 2WD](http://img.youtube.com/vi/4y71n7J57z8/0.jpg)](http://www.youtube.com/watch?v=4y71n7J57z8)

**Environment**

* bash shell
* Overture FMU exporter https://github.com/overturetool/overture-fmu/releases/download/Release%2F0.2.12/fmu-import-export.jar
* Arduino Software (IDE) https://www.arduino.cc/en/Main/Software 
 * Arduino Mega libraries (included)
* avr-gcc must be avr-gcc (GCC) >= 6.3.0
* avr-g++ must be avr-g++ (GCC) >= 6.3.0
* avr-binutils 2.30
* avr-libc (2.0.0)
* avrdude

On mac: 
* Install Homebrew
* brew install avr-binutils
* brew install avr-gcc
* brew install avr-libc 
* brew install avrdude

(untested avr-gcc upgrade for windows and linux http://blog.zakkemble.co.uk/avr-gcc-builds/)

## Quick guide

This is a all in one guide with minimal explination. It assumes that the tools are nativily installed on a *nix system. You need to provide the path when encoded with `<>`.

You must also have the arduino ide downloaded and locate the sub path within it `hardware/arduino/avr` pointing to the root of the library files which must be provided in the compile script.

Execute the following in the location of the folder containing the shell script: `deploy-backend-mega.sh`

```bash
# Download Overture FMU exporter
wget https://github.com/overturetool/overture-fmu/releases/download/Release%2F0.2.12/fmu-import-export.jar
# Export model
java -jar fmu-import-export.jar -export source -name lf -output . -root <path to model root where *.vdmrt files are located>
# Compile and upload to target
port=/dev/<serial device name> avr=<path to avr in arduino ide> ./deploy-backend-mega.sh lf.fmu
```


## Model Export to C code

To export the model we need to know its location (`root` is the path to the root of the VDM model), and then call the exporter as follows:

```bash
java -jar fmu-import-export.jar -export source -name lf -output . -root LFRController
```

The result is a new file named `lf.fmu` which is a zip archieve that contains the C source code.


## Compliation

Please set the following if not installed in the path:
* `avr` to the location of the avr libraries in the Arduino IDE. For macOS this is:

```bash
avr=/Applications/Arduino.app/Contents/Java/hardware/arduino/avr
```
* set `port` to the path to the serial port of the Arduino Mega
* `avrbin` to `/Applications/Arduino.app/Contents/Java/hardware/tools/avr/bin/`

To compile and deploy the source code FMU call the deploy script as follows (`lf.fmu` is what is previously exported):

```bash
port=/dev/cu.usbmodem1421 avr=/Applications/Arduino.app/Contents/Java/hardware/arduino/avr ./deploy-backend-mega.sh lf.fmu 
```

The output should show something similar to this for the compilation:

```bash

AVR Memory Usage
----------------
Device: atmega2560

Program:   59460 bytes (22.7% Full)
(.text + .data + .bootloader)

Data:       1514 bytes (18.5% Full)
(.data + .bss + .noinit)

```

and for deployment:

```bash
Deploying...

avrdude: Version 6.3, compiled on Jan 17 2017 at 12:01:35
         Copyright (c) 2000-2005 Brian Dean, http://www.bdmicro.com/
         Copyright (c) 2007-2014 Joerg Wunsch


avrdude: AVR device initialized and ready to accept instructions

Reading | ################################################## | 100% 0.01s

avrdude: Device signature = 0x1e9801 (probably m2560)

Writing | ################################################## | 100% 9.55s
Reading | ################################################## | 100% 7.62s

avrdude: verifying ...
avrdude: 59460 bytes of flash verified

avrdude: safemode: hfuse reads as D8
avrdude: safemode: efuse reads as FD
avrdude: safemode: Fuses OK (E:FD, H:D8, L:FF)

avrdude done.  Thank you.

```

# Robot Test

To test the robot platform please use the provided `robot-test.ino` Arduino file. It can be uploaded using Arduino IDE while selecting Arduino Mega 2560 as the board.

After upload open a serial port using baudrate 115200 and send `h` to obtain the menu. Use it to check the sensor readings and to check that the motor controller working correctly.

