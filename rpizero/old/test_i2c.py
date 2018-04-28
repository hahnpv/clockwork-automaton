#!/usr/bin/python
import smbus2
import time
import struct
import numpy as np

# for RPI version 1, use "bus = smbus.SMBus(0)"
bus = smbus2.SMBus(1)

# This is the address we setup in the Arduino Program
address = 0x44

#try:
for i in range(255):
  time.sleep(1)
  print i
  bus.write_byte_data(address, 0, i);
#except:
#  print "caught exception!"
