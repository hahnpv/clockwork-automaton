#!/usr/bin/python

import smbus
import math

# Power management registers
power_mgmt_1 = 0x6b
power_mgmt_2 = 0x6c

def read_byte(adr):
    return bus.read_byte_data(address, adr)

def read_word(adr):
    high = bus.read_byte_data(address, adr)
    low = bus.read_byte_data(address, adr+1)
    val = (high << 8) + low
    return val

def read_word_2c(adr):
    val = read_word(adr)
    if (val >= 0x8000):
        return -((65535 - val) + 1)
    else:
        return val

def dist(a,b):
    return math.sqrt((a*a)+(b*b))

def get_y_rotation(x,y,z):
    radians = math.atan2(x, dist(y,z))
    return -math.degrees(radians)

def get_x_rotation(x,y,z):
    radians = math.atan2(y, dist(x,z))
    return math.degrees(radians)

bus = smbus.SMBus(1) # or bus = smbus.SMBus(1) for Revision 2 boards
address = 0x68       # This is the address value read via the i2cdetect command

# Now wake the 6050 up as it starts in sleep mode
bus.write_byte_data(address, power_mgmt_1, 0)

while True:
  gx = (read_word_2c(0x43) / 131.0)
  gy = (read_word_2c(0x45) / 131.0)
  gz = (read_word_2c(0x47) / 131.0)

  print "gyro: ", gx, gy, gz

  ax = read_word_2c(0x3b) / 16384.0
  ay = read_word_2c(0x3d) / 16384.0
  az = read_word_2c(0x3f) / 16384.0

  print "accel: ", ax, ay, az

  # calculate accelerometer angles
  arx = (180/3.141592) * math.atan(ax / math.sqrt(ay**2 + az**2))
  ary = (180/3.141592) * math.atan(ay / math.sqrt(ax**2 + az**2))
  arz = (180/3.141592) * math.atan(math.sqrt(ay**2 + ax**2) / az)
