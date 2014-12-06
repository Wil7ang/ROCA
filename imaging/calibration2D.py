import sys

sys.path.insert(0,"/opt/local/lib/python2.7/site-packages/")
sys.path.insert(0,"/opt/local/lib")

import numpy as np
import cv2



calibrationResolution = 35 #Calibration resolution in centimeters
calibrationMinimum = -70
calibrationMaximum = 70

currentRealX = calibrationMinimum #Real X in centimeters
currentRealZ = 0 #Real Z in centimeters

calibrationItemX = 0
calibrationItemZ = 0

calibrationArray = [[]]

pixelX = 0
pixelY = 0
pixelZ = 0

screenX1 = 0
screenY1 = 0
screenX2 = 0
screenY2 = 0

def onMouse2(event, x, y, flags, param):
    if event == cv2.EVENT_LBUTTONDOWN:
        global screenX1,screenX2,screenY1,screenY2
        screenX2 = x-640
        screenY2 = y
        print "Camera2 " + str(x) + " " + str(y)

def onMouse(event, x, y, flags, param):
    if event == cv2.EVENT_LBUTTONDOWN and x <= 640:
        global screenX1,screenX2,screenY1,screenY2
        screenX1 = x
        screenY1 = y
        print "Camera1 " + str(x) + " " + str(y)
    else:
        onMouse2(event,x,y,flags,param)


CAMERAWIDTH = 640
CAMERAHEIGHT = 480

camera1 = cv2.VideoCapture(2)
camera2 = cv2.VideoCapture(0)
camera1.set(3,CAMERAWIDTH)
camera1.set(4,CAMERAHEIGHT)
camera2.set(3,CAMERAWIDTH)
camera2.set(4,CAMERAHEIGHT)

cv2.namedWindow('frame1')
cv2.namedWindow('frame2')
cv2.setMouseCallback('frame1', onMouse)
cv2.setMouseCallback('frame2', onMouse2)


while(True):
    ret,frame1 = camera1.read()
    ret,frame2 = camera2.read()

    both = np.hstack((frame1,frame2))

    cv2.circle(both, (screenX1, screenY1), 2, (0,0,255), 2)
    cv2.circle(both, (screenX2+640, screenY2), 2, (0,0,255), 2)

    cv2.imshow('frame1',both)
    #cv2.imshow('frame2',frame2)

    keyhit = cv2.waitKey(1) & 0xFF
    if keyhit == ord('q'):
        break
    elif keyhit == ord('n'):
        calibrationArray[calibrationItemZ].append((screenX1,screenY1,screenX2,screenY2))

        currentRealX += calibrationResolution
        calibrationItemX += 1
        if(currentRealX > calibrationMaximum):
            calibrationItemX = 0
            calibrationItemZ += 1

            currentRealX = calibrationMinimum
            currentRealZ += calibrationResolution

            if(currentRealZ > calibrationMaximum):
                break
            else:
                calibrationArray.append([])

        print "Now calibrating: " + str(currentRealX) + ", " + str(currentRealZ)


for z in calibrationArray:
    for x in z:
        print x,
    print '\n'

print "Point3DD GetRealWorldCoordinates(int X1, int Y1, int X2, int Y2)"
print "{"
print " double realX = 0.0;"
print " double realY = 35.0;"
print " double realZ = 0.0;"

for z in range(0,len(calibrationArray)-1):
    for x in range(0,len(calibrationArray[z])-1):
        print " // " + str(x) + ", " + str(z) + " to " + str(x+1) + ", " + str(z+1)
        print " if(X1 > " + str(calibrationArray[z][x][0]) + " && X1 <= " + str(calibrationArray[z][x+1][0]) + " && Y1 > " + str(calibrationArray[z+1][x][1]) + " && Y1 <= " + str(calibrationArray[z][x+1][1]) \
            + " && Y2 <= " + str(calibrationArray[z][x][3]) + " && Y2 > " + str(calibrationArray[z][x+1][3]) + ")"
        print " {"
        print "     realX = ((" + str(calibrationArray[z][x][3]) + ".0 - (double)Y2)/" + str(calibrationArray[z][x][3] - calibrationArray[z][x+1][3]) + ".0 * " + str(calibrationResolution) + ".0 + " + str(x * calibrationResolution) + ".0) - 70.0;"
        print "     double adjustedY1 = (double)Y1 - (" + str(calibrationArray[z][0][3]) + ".0 - (double)Y2)/(" + str(calibrationArray[z][0][3] - calibrationArray[z][len(calibrationArray[z])-1][3]) + ".0) * " + str(calibrationArray[z][len(calibrationArray[z])-1][1] - calibrationArray[z][0][1]) + ".0;"
        print "     realZ = (((" + str(calibrationArray[0][0][1]) +  " - adjustedY1)/" + str(calibrationArray[0][0][1] - calibrationArray[1][0][1]) + ") * " + str(calibrationMaximum/2) + ".0);"
        print "     return Point3DD(realX,realY,realZ);"
        print " }"
        print '\n'
print "}"


camera1.release()
camera2.release()
cv2.destroyAllWindows()