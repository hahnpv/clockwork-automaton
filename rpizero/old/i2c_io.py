#!/usr/bin/python
import smbus2
import time
import struct

# for RPI version 1, use "bus = smbus.SMBus(0)"
bus = smbus2.SMBus(1)

# This is the address we setup in the Arduino Program
address = 0x04

def write_block(flipper, left, right):
    data = [flipper, left, right]
    bus.write_i2c_block_data(address, 0, data)
    print data

def read_block(num_bytes):
    vals = bus.read_i2c_block_data(address, 0,  num_bytes)
    return vals

while True:
    var = input("")
    if not var:
        continue
    write_block(110,var,var);
    temp = read_block(7);
    print temp
