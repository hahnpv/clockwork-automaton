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

leftDelta = 25
rightDelta = 15
leftTimer = 0
rightTimer = 0
right = 90 + rightDelta
left  = 90 + leftDelta
write_block(110,left,right)
try:
  while True:
    time.sleep(0.005)
    temp = read_block(7);
    if np.any(temp[0:3]):
#      print temp
      if np.any(temp[0]):
        # FRONT RIGHT
        right = 90-rightDelta;
        rightTimer = time.time()
      if np.any(temp[1]):
        # FRONT LEFT
        left = 90-leftDelta;
        leftTimer = time.time()
      if np.any(temp[2]):
        # REAR RIGHT
        right = 90+rightDelta;
      if np.any(temp[3]):
        # REAR LEFT
        left = 90+leftDelta;
      write_block(110,left,right)
    else:
      if time.time()-leftTimer > 0.5 and leftTimer<>0:
        left = 90 + leftDelta
        write_block(110,left,right)
        leftTimer = 0
      if time.time()-rightTimer > 0.5 and rightTimer<>0:
        right = 90 + rightDelta
        write_block(110,left,right)
	rightTimer = 0
except:
  print "caught exception!"
  write_block(90,90,90)
