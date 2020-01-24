# Hardware overview

The new linefollower robot is made up of 3 boards:

1. An Arduino Uno that drives the servos and is connected to the sensors. It is controllable using I2C or GPIO PINS based on the hardware switch which desides the mode.
2. An ATMega128P which is also connected to the sensors and has two GPIO connected to the Arduino for simple control of the servos
3. An Raspberry PI that is connected though a level converter to the Arduino I2C where it reads the sensors and sets the PWM signals for the servos

Due to the high power load batteries (4 pices) does not last very long, and control will be schetcy when power is running low.

Currently there is not a good way to power the robot directly from a wall socket.

It would be good to upgrade the power supply such that the external power connector that feds power to the battery rail could be connected to a 12V power supply delivering sufficient power to the Arduino (which can handle 12V) however a e.g. 2A DC Step Down 5V Voltage Regulator Power Supply Converter module from 12v. This way everything will have more than enough power. The Arduino can not be powered by 5V from a USB charger since it requires 9-12V.

