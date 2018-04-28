#!/usr/bin/python
import picamera
import glob
import time
with picamera.PiCamera() as camera:
    i = 0
    j = 0
    template = "motion%03d"
    serial   = "_%03d"
    while True:
        if not glob.glob((template + '*') % j):
            break
        j += 1
    print i,j
    i = 0
#    camera.resolution = (640, 480)
    camera.resolution = (1296, 972)
#    camera.resolution = (1920,1080)
    camera.framerate = 30		# 20Hz is what my controls are currently using
    camera.start_recording((template + serial + '.h264') % (j,i), motion_output=(template + serial + '.data') % (j,i))
    try:
#        while True:
#            camera.wait_recording(30);
#            i += 1;
#            camera.split_recording((template + serial + '.h264') % (j,i), motion_output=(template + serial + '.data') % (j,i))
      time.sleep(30)
    except KeyboardInterrupt:
        print "exiting"
    camera.stop_recording()
