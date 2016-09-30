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
#
import time
import serial

#Be sure the serial port is closed before running.
comPort = "COM5" #Change this according to where the Teensy device is connected

currentTime = time.time()
previousTime = currentTime

timeZoneOffset = 0

#Press Ctrl-C to exit the loop
while True:
    currentTime = time.time()
    if (currentTime - previousTime >= 1): #Run the following every second
        previousTime = currentTime
        with serial.Serial(comPort) as ser:
            
            ser.write(b'T')
            ser.write(bytes("T%d\r\n" %(currentTime-timeZoneOffset*3600),'ascii'))
            
            print(ser.readline()) #Should display the time of the Teensy as bytes
