# arduino
_Some sketches for Arduino small devices_

### AWS_M5_control.ino
Subscribe to Amazon AWS IoT topic from [M5Stack](https://m5stack.com/) ESP32 device. It expects to receive message in JSON format with one 'data' floating point value: { "data" : 12.345 }. Then recevied value is printed on a screen and written to a text file on SD card. 

### Came_Gates_Conrol.ino
Sketch for small USB-powered device to open Came gates with known codes - you need to put your codes in file after `// Defind gate codes` line. (You may use RCSwitch library to sniff gate code from air - it works fine for reception but I failed to use it for transmission so custom function is used).

Device itself consists of Arduino Nano, 3x4 Keypad and 433 MHz transmitter. Each button have a 24-bit code assigned - this code is send via @433 MHz with Came-gate encoding. Pins from 3 to 9 are connected to Keypad, pin 11 is connected to trasmitter DATA pin. Transmitted is powered from GND and +5V pins of Arduino.

### M5_Came_Scanner.ino
This scetch uses [M5Stack](https://m5stack.com/) device and 433 MHz receiver module connected to Pin 2 together with modified RCSwitch arduino library (see first lines of source code about library modifications).

It sniffs 433 MHz transmission for Came gate codes - if code is received it is stored in internall 12-items buffer snd higlighted on the screen.
