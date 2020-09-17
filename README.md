# arduino
_Some sketches for Arduino small devices_


### Came_Gates_Conrol.ino
Sketch for small USB-powered device to open Came gates with known codes - you need to put your codes in file after `// Defind gate codes` line. (You may use RCSwitch library to sniff gate code from air - it works fine for reception but I failed to use it for transmission so custom function is used).

Device itself consists of Arduino Nano, 3x4 Keypad and 433 MHz transmitter. Each button have a 24-bit code assigned - this code is send via @433 MHz with Came-gate encoding. Pins from 3 to 9 are connected to Keypad, pin 11 is connected to trasmitter DATA pin. Transmitted is powered from GND and +5V pins of Arduino.
