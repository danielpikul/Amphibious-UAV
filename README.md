# Amphibious UAV

You can find lots of development information at the [ArduPilot development site](http://dev.ardupilot.com)

#### To compile APM2.x Ardupilot after version 3.1 please follow the instructions found at 

[Dev.Ardupilot] (http://dev.ardupilot.com/wiki/building-ardupilot-with-arduino-windows/) 


```

## Prerequisites


## Building using the Arduino IDE

ArduPilot is no longer compatible with the standard Arduino
distribution.  You need to use a patched Arduino IDE to build
ArduPilot.

Do not try to use the Arduino IDE to build in Linux--you should follow
the instructions in the "Building using make" section.

1. The patched ArduPilot Arduino IDE is available for Mac and Windows
   from the [downloads
   page](http://firmware.diydrones.com).

2. Unpack and launch the ArduPilot Arduino IDE. In the preferences
   menu, set your sketchbook location to your downloaded or cloned
   `ardupilot` directory.

3. In the ArduPilot Arduino IDE, select your ArduPilot type (APM1 or
   APM2) from the ArduPilot menu (in the top menubar).

4. Restart the ArduPilot Arduino IDE. You should now be able to build
   ArduCopter from source.

5. Remember that, after changing ArduPilot type (APM1 or APM2) in the
   IDE, you'll need to close and restart the IDE before continuing.


