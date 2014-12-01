import sys

sys.path.insert(0,"/usr/local/lib/python2.7/site-packages")
sys.path.insert(0,"/usr/local/lib")

import numpy as np
import cv2

import time
import datetime

def detectMotion(motion, result, x_start, x_stop, y_start, y_stop, max_deviation, color):
    mean, stddev = cv2.meanStdDev(motion)

    if(stddev[0] < max_deviation):
        number_of_changes = 0
        min_x = motion.shape[1]
        max_x = 0
        min_y = motion.shape[0]
        max_y = 0

        for j in range(y_start,y_stop-1,2):
            for i in range(x_start,x_stop-1,2):
                if(np.take(motion,j*90+i) == 255):
                    number_of_changes += 1
                    if(min_x>i): min_x = i
                    if(max_x<i): max_x = i
                    if(min_y>j): min_y = j
                    if(max_y<j): max_y = j

        if(number_of_changes):
            if(min_x-10 > 0): min_x -= 10
            if(min_y-10 > 0): min_y -= 10
            if(max_x+10 < result.shape[1]-1): max_x += 10
            if(max_y+10 < result.shape[0]-1): max_y += 10

            # x = (min_x,min_y)
            # y = (max_x,max_y)
            # rect = (x,y)
            # cv2.rectangle(result,rect,color,1)

            return (min_x+(max_x-min_x)/2,min_y+(max_y-min_y)/2)
    else:
        return (0,0)

def determinant3x3(a, b, c,
                   d, e, f,
                   g, h, i):
    return a*e*i + b*f*g + c*d*h - a*f*h - b*d*i - c*e*g

CAMERAWIDTH = 640
CAMERAHEIGHT = 480

def main():
    camera1 = cv2.VideoCapture(0)
    camera2 = cv2.VideoCapture(1)

    ret, result = camera1.read()
    ret2, result2 = camera2.read()

    prev_frame = cv2.cvtColor(result, cv2.COLOR_BGR2GRAY)
    current_frame = cv2.cvtColor(result, cv2.COLOR_BGR2GRAY)
    next_frame = cv2.cvtColor(result, cv2.COLOR_BGR2GRAY)

    prev_frame2 = cv2.cvtColor(result2, cv2.COLOR_BGR2GRAY)
    current_frame2 = cv2.cvtColor(result2, cv2.COLOR_BGR2GRAY)
    next_frame2 = cv2.cvtColor(result2, cv2.COLOR_BGR2GRAY)

    number_of_changes = number_of_sequence = 0;

    mean_ = cv2.cv.Scalar(0)
    color = cv2.cv.Scalar(0,255,255)

    x_start = 0
    x_stop = CAMERAWIDTH
    y_start = 0
    y_stop = 340

    x_start2 = 200
    x_stop2 = CAMERAWIDTH-200
    y_start2 = 0
    y_stop2 = CAMERAHEIGHT

    max_deviation = 80

    kernel_ero = cv2.getStructuringElement(cv2.MORPH_RECT, (2,2))

    pointStorage = []
    pointStorage2 = []

    gotThreePoints = False

    count = 0
    while(True):
        dt1 = datetime.datetime.now()

        if(count%2 == 0):
            prev_frame = current_frame
            current_frame = next_frame

            prev_frame2 = current_frame2
            current_frame2 = next_frame2

            ret, next_frame = camera1.read()
            time.sleep(0.02)
            ret2, next_frame2 = camera2.read()

            result = next_frame
            result2 = next_frame2
            next_frame = cv2.cvtColor(next_frame, cv2.COLOR_BGR2GRAY)
            next_frame2 = cv2.cvtColor(next_frame2, cv2.COLOR_BGR2GRAY)


        d1 = cv2.absdiff(prev_frame, next_frame)
        d2 = cv2.absdiff(next_frame, current_frame)
        motion = cv2.bitwise_and(d1, d2)
        retval, motion = cv2.threshold(motion, 20, 255, cv2.cv.CV_THRESH_BINARY)
        motion = cv2.erode(motion, kernel_ero)

        d12 = cv2.absdiff(prev_frame2, next_frame2)
        d22 = cv2.absdiff(next_frame2, current_frame2)
        motion2 = cv2.bitwise_and(d12, d22)
        retval, motion2 = cv2.threshold(motion2, 20, 255, cv2.cv.CV_THRESH_BINARY)
        motion2 = cv2.erode(motion2, kernel_ero)

        pointTemp = detectMotion(motion, result,  x_start, x_stop, y_start, y_stop, max_deviation, color)
        # pointTemp2 = detectMotion(motion2, result2,  x_start2, x_stop2, y_start2, y_stop2, max_deviation, color)

        # if(pointTemp and pointTemp2 and pointTemp2[0] !=0 and pointTemp2[1] !=0 and pointTemp[0] !=0 and pointTemp[1] !=0):
        #     pointStorage2.append(pointTemp2)
        #     pointStorage.append(pointTemp)

        # for i in range(0,len(pointStorage)):
        #     if(i > 0):
        #         cv2.line(result, (pointStorage[i][0],pointStorage[i][1]),(pointStorage[i-1][0],pointStorage[i-1][1]), cv2.cv.Scalar(0,0,255,1), 2)
        #     cv2.circle(result, (pointStorage[i][0],pointStorage[i][1]), 3, cv2.cv.Scalar(255,0,0,1), 2)

        # for i in range(0,len(pointStorage2)):
        #     if(i > 0):
        #         cv2.line(result, (pointStorage2[i][0],pointStorage2[i][1]),(pointStorage2[i-1][0],pointStorage2[i-1][1]), cv2.cv.Scalar(0,0,255,1), 2)
        #     cv2.circle(result, (pointStorage2[i][0],pointStorage2[i][1]), 3, cv2.cv.Scalar(255,0,0,1), 2)


    #     if(count%2 == 0):
    #         ret,frame1 = camera1.read()
    #         time.sleep(0.02)
    #         ret2,frame2 = camera2.read()

    #     # Our operations on the frame come here
    #     gray1 = cv2.cvtColor(frame1, cv2.COLOR_BGR2GRAY)
    #     gray2 = cv2.cvtColor(frame2, cv2.COLOR_BGR2GRAY)

        # Display the resulting frame
        cv2.imshow('frame1',result)
        cv2.imshow('frame2',result2)
        if cv2.waitKey(1) & 0xFF == ord('q'):
          break

        count += 1
        if(count >= 30):
            count = 0
        dt2 = datetime.datetime.now()
        # print (dt2-dt1).microseconds

    # # When everything done, release the capture
    camera1.release()
    camera2.release()
    cv2.destroyAllWindows()

if __name__ == "__main__":
    main()