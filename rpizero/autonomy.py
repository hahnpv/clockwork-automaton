#!/usr/bin/python
import time
import glob

# SCREEN
import Adafruit_GPIO.SPI as SPI
import Adafruit_SSD1306
from PIL import Image
from PIL import ImageDraw
from PIL import ImageFont
RST = 24
disp = Adafruit_SSD1306.SSD1306_128_64(rst=RST)
disp.begin()
disp.clear()
disp.display()
width = disp.width
height = disp.height
image = Image.new('1', (width, height))
draw = ImageDraw.Draw(image)
draw.rectangle((0,0,width,height), outline=0, fill=0)
font = ImageFont.load_default()
draw.rectangle((0,0,width,height), outline=0, fill=0)
draw.text((2, 18),'AUTONOMOUS DISABLE',  font=font, fill=255)
disp.image(image)
disp.display()

# CAMERA
import picamera
camera = picamera.PiCamera()
camera.resolution = (1296, 972)	# (1920, 1080)
camera.framerate = 30

def find_filename():
  i = 0
  j = 0
  while True:
    if not glob.glob(('motion%03d*') % j):
      break
    j += 1
  return j,i

# LIDAR
import serial
serial = serial.Serial("/dev/ttyAMA0", 115200)
file = []
def packet(dati):
  if len(dati)==21:
    ##index data packets go from 0xA0 (160) to 0xF9(359). Subtract 160 to normalize scale of data packets from 0 to 90.
    dati[0]=((dati[0])-160)
    for i in (1,2,3,4):
      if dati[i*4] != 128:
        dist_mm = dati[4*i-1] | (( dati[4*i] & 0x3f) << 8)
        angle = dati[0]*4+i+1
        file.write(str(angle) + ' ' + str(dist_mm) + '\n')
        file.flush()
#        print (angle,dist_mm)


# GPIO
import RPi.GPIO as GPIO
GPIO.setmode(GPIO.BCM)
GPIO.setup(17,GPIO.IN)

# I2C
import smbus2
bus = smbus2.SMBus(1)
address = 0x44

# LOCAL
latch = 0 			# latch state
screen = False 			# screen state
index, slice = find_filename()  # unique filename and slice
b = 0                           # predeclare persistent variable

try:
  while(1):
  #  time.sleep(1)
    latch = GPIO.input(17)
  #  print "Latch: " + str(latch) + " screen: " + str(screen)
    if latch == 1 and screen is False:
      camera.start_recording(('motion%03d_%03d.h264') % (index,slice), motion_output=('motion%03d_%03d.data') % (index,slice))
      file = open(('lidar%03d_%03d.csv') % (index,slice),'w')
      draw.rectangle((0,0,width,height), outline=0, fill=0)
      draw.text((2, 2),'AUTONOMOUS ENABLE',  font=font, fill=255)
      disp.image(image)
      disp.display()
      screen = True
    if latch == 0 and screen is True:
      camera.stop_recording()
      file.flush()
      file.close()
      slice = slice + 1;
      draw.rectangle((0,0,width,height), outline=0, fill=0)
      draw.text((2, 18),'AUTONOMOUS DISABLE',  font=font, fill=255)
      disp.image(image)
      disp.display()
      screen = False
    if latch == 1:
      # any time we are latched, get lidar
      b = (ord(serial.read(1))) ##initial read
      dati = []
      while True:
        ##250 == FA, FA is the start value - it's constant
        ##Each data packet is 22 bytes, > 20 means len(dati) == at least 21
        if b==(250) and len(dati)>20:
          break
        dati.append(b)             # append to list
        b = (ord(serial.read(1)))  # read again
        time.sleep(0.0001)	 # prevent python from hogging cpu
      packet(dati)
  # CALC TARGET
  # bus.write_byte_data(address, 0, BAM);
except:
  camera.stop_recording()
  file.flush()
  file.close()
  draw.rectangle((0,0,width,height), outline=0, fill=0)
  draw.text((2, 18),'AUTONOMOUS DISABLE',  font=font, fill=255)
  disp.image(image)
  disp.display()

