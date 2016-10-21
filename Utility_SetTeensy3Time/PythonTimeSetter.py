#/usr/env/python3
#
# Run this program to send a time value to the Teensy3 USB development board
# The Teensy should be running the example code in the following Arduino menu:
# File -> Examples -> Time -> TimeTeensy3
#
# This script will also work with the SetTeensy3Time.ino sketch.
#
# Written by Dr. Jeremy S. Daily
# The University of Tulsa
# Department of Mechanical Engineering
#
# Released under the MIT License
#
# Copyright (c) 2016 Jeremy S. Daily
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
# 
# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.
# 
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.
#
# Depends on PySerial:
# https://github.com/pyserial/pyserial

import time
import serial
import serial.tools.list_ports

TeensyFound = False

for possibleCOMPort in serial.tools.list_ports.comports():
    print(possibleCOMPort)
    if ('Teensy' in str(possibleCOMPort)):
        print("Found Teensy! Trying to set time.")
        comPort = str(possibleCOMPort)[0:5] #Gets the first digits
        TeensyFound = True
        break


if TeensyFound:
    print("Press Ctrl-C to exit")
    print("Be sure all programs with COM ports connected to the Teensy are closed.")

    currentTime = time.time()
    previousTime = currentTime

    timeZoneOffset = 0
   
    while TeensyFound:
        currentTime = time.time()
        if (currentTime - previousTime >= 1): #Run the following every second
            previousTime = currentTime
            with serial.Serial(comPort) as ser:
                
                ser.write(b'T')
                ser.write(bytes("T%d\r\n" %(currentTime-timeZoneOffset*3600),'ascii'))
                
                print(ser.readline()) #Should display the time of the Teensy as bytes
else:
    print("Teensy Not Found. Please plug in and make sure drivers have installed.")
