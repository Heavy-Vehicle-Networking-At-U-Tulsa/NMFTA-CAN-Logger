# NMFTA CAN Data Collection
A repository supporting the low cost controller area network (CAN) logger devices commissioned for the National Motor Freight Traffic Association

The source code that was compiled and installed on the NMFTA CAN loggers delivered on 14 Nov 2016 is the directory titled  _07_Low_Latency_Logger_with_Requests with the git commit sha of e10ae59f31be61553c1f9859fdde8c01946b17c0.

Supporting website: https://heavy-vehicle-networking-at-u-tulsa.github.io/NMFTA-CAN-Logger/

## Project Description
The NMFTA data collection effort will collect CAN and J1939 data from heavy vehicles being used on the road. The goal is to gather enough data to be able to develop a baseline picture of normal CAN traffic. These data sets can be used for researching methods and algorithms to detect anomalous behavior. The project is open source with both hardware and software. Students at the University of Tulsa significantly contributed to the delivery of these logging devices. 

## Open Source License
Unless otherwise noted or conflicting, the contents of this repository are released under the MIT license:

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software, hardware and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE OR HARDWARE.

## Using the Software
To run the software in this repository, you will need the following:
* Arduino 1.6.11
* Teensyduino 1.30
* Python 3.4
* PySerial
* Matplotlib

##About the Source Code
The enumerated folders contain Arduino sketches that compile using the Teensyduino extention. Teensyduino is installed to augment the Arduino programming environment. The sketches follow the development progression and test basic functionality. They build on one another and each bring in different functionality. Many sketches are based off library examples and modified to suit the CAN Logger hardware.

Some extra libraries are needed to make use of the hardware on the NMFTA board.

## Hardware
The foundation of the hardware is the Teensy 3.2 Arduino compatible USB development board with a 32-bit ARM processor (Freescale MK20D). The Teensy 3.2 has a built in CAN controller that is accessed using the FlexCAN library. An additional CAN channel is added by including an integrated MCP25625 controller with tranceiver. This integrated device gives the system a second CAN channel. The additional CAN controller communicates with the Teensy 3.2 using SPI. Also, the micro SD card is accessed with SPI communication. When writing code to support these peripherials, ensure the chip select pins are pulled high to enable proper hardware addressing. 

The realtime clock built into the Teensy's processor requires an external crystal and battery backup to work correctly. Setting the realtime clock can be done by sending a serial string starting with the letter "T" using a PC. To do this, a simple Python script is included in the Utilities folder that finds a Teensy that is plugged into the computer and sends it the serial string with the correct time. 

A configuration switch is included that can electrically combine the different CAN channels. This feature enables a network to be separated or combined when the device is installed inline between two CAN nodes. If the lines are separated, then termination resistors are available to reestablish the correct CAN network impedence.

Green and red LEDs are available on the board for a visual indication of the mode of operation. These LEDs can be seen through the clear heatshrink that wraps the board.


