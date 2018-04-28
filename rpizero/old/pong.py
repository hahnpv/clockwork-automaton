#!/usr/bin/python
import smbus2
import time
import struct
import numpy as np

# for RPI version 1, use "bus = smbus.SMBus(0)"
bus = smbus2.SMBus(1)

# This is the address we setup in the Arduino Program
address = 0x04

def write_block(flipper,left,right):
  data = [flipper, right, left]
  bus.write_i2c_block_data(address, 0, data)

def read_block(num_bytes):
  vals = bus.read_i2c_block_data(address, 0,  num_bytes)
  return vals

leftDelta = 12
rightDelta = 10
right = 90 + rightDelta
left  = 90 + leftDelta
write_block(135,left,right)
try:
  while True:
    time.sleep(0.02)
    temp = read_block(7);
    dirty = False
    if np.any(temp[0:3]):
      if np.any(temp[0]) and right > 90:
        # FRONT RIGHT
        right = 90-rightDelta;
        dirty = True
      if np.any(temp[1]) and left > 90:
        # FRONT LEFT
        left = 90-leftDelta;
        dirty = True
      if np.any(temp[2]) and right < 90:
        # REAR RIGHT
        right = 90+rightDelta;
        dirty = True
      if np.any(temp[3]) and left < 90:
        # REAR LEFT
        left = 90+leftDelta;
        dirty = True
      if dirty:
        write_block(135,left,right)
except:
  print "caught exception!"
  write_block(90,90,90)
