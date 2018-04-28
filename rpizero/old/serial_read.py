#!/usr/bin/env python
import time
import serial
ser = serial.Serial(port='/dev/ttyAMA0',baudrate = 115200,timeout=3.0)
counter=0

while 1:
    x=ser.read(4)
    print x

