import cv2
import os.path
import numpy as np
import matplotlib.pyplot as plt
import time

#inspired/derived from http://www.kscottz.com/dewarped-panoramic-images-from-a-raspberrypi-camera-module/
#using pure opencv

# build the mapping
def buildMap(Ws,Hs,Wd,Hd,R1,R2,Cx,Cy):
    mapx = np.zeros((Hd,Wd),np.float32)
    mapy = np.zeros((Hd,Wd),np.float32)
    wd = 1. / float(Wd)
    hd = 1. / float(Hd)
    for y in range(0,int(Hd-1)):
        for x in range(0,int(Wd-1)):
            r = (float(y) * hd)*(R2-R1)+R1
            theta = (float(x) * wd)*2.0*np.pi
            xS = Cx-r*np.sin(theta)				# +sin
            yS = Cy-r*np.cos(theta)				# +cos
            mapx[y,x] = int(xS)
            mapy[y,x] = int(yS)
    np.savez('cammap', mapx=mapx, mapy=mapy)
    return mapx, mapy

makeMap = True
if makeMap:
    # center of the "donut"
    Cx = 575
    Cy = 465
    # Inner donut radius
    R1 = 140
    # outer donut radius
    R2 = 280
    # our input and output image siZes
    Wd = np.int32(2.0*((R2+R1)/2)*np.pi)
    Hd = (R2-R1)
    Ws = 1296
    Hs = 972
    # build the pixel map, this could be sped up
    print "BUILDING MAP!"
    tic = time.time()
    xmap,ymap = buildMap(Ws,Hs,Wd,Hd,R1,R2,Cx,Cy)
    print time.time() - tic
    print "MAP DONE!"
else:
    maps = np.load('cammap.npz')
    print maps.files
    xmap = maps['mapx']
    ymap = maps['mapy']

filename = 'motion005_000.h264'
print os.path.isfile(filename)
cap = cv2.VideoCapture(filename)
time.sleep(2)
success,image = cap.read()
cv2.imwrite('image.png',image)
print success
fourcc = cv2.VideoWriter_fourcc(*'MP4V')
out = cv2.VideoWriter('dewarped.mp4', fourcc, 30.0, (Wd,Hd))
while(success):
    result = cv2.remap(image,xmap,ymap,cv2.INTER_LINEAR) 
    out.write(result)
    cv2.imshow('frame',result)
    success, image = cap.read()
#    if success:
    if (cv2.waitKey(1) & 0xFF == ord('q')):
        break      
cap.release()
out.release()
cv2.destroyAllWindows()
