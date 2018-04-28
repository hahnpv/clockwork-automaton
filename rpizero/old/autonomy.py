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
draw.text((2, 2),'AUTONOMOUS DISABLE',  font=font, fill=255)
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

# I/O
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

while(1):
  time.sleep(1)
  latch = GPIO.input(17)
  print "Latch: " + str(latch) + " screen: " + str(screen)
  if latch == 1 and screen is False:
    camera.start_recording(('motion%03d_%03d.h264') % (index,slice), motion_output=('motion%03d_%03d.data') % (index,slice))
    draw.rectangle((0,0,width,height), outline=0, fill=0)
    draw.text((2, 2),'AUTONOMOUS ENABLE',  font=font, fill=255)
    disp.image(image)
    disp.display()
    screen = True
  if latch == 0 and screen is True:
    camera.stop_recording()
    slice = slice + 1;
    draw.rectangle((0,0,width,height), outline=0, fill=0)
    draw.text((2, 2),'AUTONOMOUS DISABLE',  font=font, fill=255)
    disp.image(image)
    disp.display()
    screen = False

  # CALC TARGET
  # bus.write_byte_data(address, 0, BAM);
