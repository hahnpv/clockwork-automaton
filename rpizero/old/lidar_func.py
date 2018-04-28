#!/usr/bin/python
import serial, time

ser = serial.Serial('/dev/ttyAMA0', 115200)
print(ser.name)

file = open('lidar.txt','w')

# NOTES
# 0/360 is straight forward
# might be easiest to find smallest value, greater than (min dist to walls) instead of cluster

def packet(dati):
  if len(dati)==21:
    ##index data packets go from 0xA0 (160) to 0xF9(359). Subtract 160 to normalize scale of data packets from 0 to 90.
    dati[0]=((dati[0])-160)
    for i in (1,2,3,4):
      if dati[i*4] != 128:
        dist_mm = dati[4*i-1] | (( dati[4*i] & 0x3f) << 8)
        angle = dati[0]*4+i+1
        file.write(str(angle) + ', ' + str(dist_mm) + '\n')
#        print (angle,dist_mm)


while True:
  try:
    b = (ord(ser.read(1))) ##initial read
    dati = []

    while True:
      ##250 == FA, FA is the start value - it's constant
      ##Each data packet is 22 bytes, > 20 means len(dati) == at least 21
      if b==(250) and len(dati)>20:
        break

      ##add data to list, read again
      dati.append(b)
      b = (ord(ser.read(1)))

      ##do not hog the processor power - Python hogs 100% CPU without this in infinite loops
      time.sleep(0.0001)
    packet(dati)

  except KeyboardInterrupt:
    ser.close()
    file.close()

