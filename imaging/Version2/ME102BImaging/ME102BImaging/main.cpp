#include <stdio.h>
#include <stdlib.h>
#include <cstdlib>
#include <stdint.h>
#include <unistd.h>
#include <vector>
#include <time.h>
#include <cmath>

#include "libserialport.h"

#include "opencv2/opencv.hpp"
#include "opencv2/highgui/highgui.hpp"

#include "Prediction.h"
#include "ComputeLine.h"
#include "LineLineIntersect.h"
#include "LeastSquare.h"
#include "realWorld.h"

#define CAMERAWIDTH 640
#define CAMERAHEIGHT 480
#define MAXMOTION 100

using namespace cv;
using namespace std;

//#include <unistd.h>
// Check if there is motion in the result matrix
// count the number of changes and return.
pair<int,int> detectMotion(const Mat & motion, Mat & result,
                           int x_start, int x_stop, int y_start, int y_stop,
                           int max_deviation,
                           Scalar & color)
{
    // calculate the standard deviation
    Scalar mean, stddev;
    meanStdDev(motion, mean, stddev);
    // if not to much changes then the motion is real (neglect agressive snow, temporary sunlight)
    if(stddev[0] < max_deviation)
    {
        int number_of_changes = 0;
        int min_x = motion.cols, max_x = 0;
        int min_y = motion.rows, max_y = 0;
        // loop over image and detect changes
        for(int j = y_start; j < y_stop; j+=2){ // height
            for(int i = x_start; i < x_stop; i+=2){ // width
                // check if at pixel (j,i) intensity is equal to 255
                // this means that the pixel is different in the sequence
                // of images (prev_frame, current_frame, next_frame)
                if(static_cast<int>(motion.at<uchar>(j,i)) == 255)
                {
                    number_of_changes++;
                    if(min_x>i) min_x = i;
                    if(max_x<i) max_x = i;
                    if(min_y>j) min_y = j;
                    if(max_y<j) max_y = j;
                }
            }
        }
        if(number_of_changes){
            //check if not out of bounds
            if(min_x-10 > 0) min_x -= 10;
            if(min_y-10 > 0) min_y -= 10;
            if(max_x+10 < result.cols-1) max_x += 10;
            if(max_y+10 < result.rows-1) max_y += 10;
            // draw rectangle round the changed pixel
            Point x(min_x,min_y);
            Point y(max_x,max_y);
            Rect rect(x,y);
            rectangle(result,rect,color,1);
            return pair<int,int>(min_x+(max_x-min_x)/2,min_y+(max_y-min_y)/2);
        }
    }
    return pair<int,int>(0,0);
}

void moveROCA(sp_port *connection, int16_t base, int16_t shoulder, int16_t elbow, int16_t wristA, int16_t wristB)
{
    int i = 0;
    unsigned char command[15] = {0x81, static_cast<unsigned char>((base>>8) & 0xFF), static_cast<unsigned char>(base & 0xFF),
        0x82, static_cast<unsigned char>((shoulder>>8) & 0xFF), static_cast<unsigned char>(shoulder & 0xFF),
        0x83, static_cast<unsigned char>((elbow>>8) & 0xFF), static_cast<unsigned char>(elbow & 0xFF),
        0x84, static_cast<unsigned char>((wristA>>8) & 0xFF), static_cast<unsigned char>(wristA & 0xFF),
        0x85, static_cast<unsigned char>((wristB>>8) & 0xFF), static_cast<unsigned char>(wristB & 0xFF)};
    
    for(i = 0;i<16;i+=3)
    {
        unsigned char buff[2] = {command[i],0};
        unsigned char buff2[2] = {command[i+1],0};
        unsigned char buff3[2] = {command[i+2],0};
        
        sp_blocking_write(connection, buff, 1, 100);
        usleep(10000);
        sp_blocking_write(connection, buff2, 1, 100);
        usleep(10000);
        sp_blocking_write(connection, buff3, 1, 100);
        usleep(10000);
    }
}


int X1,Y1,X2,Y2;
bool camera1Set = false;
bool camera2Set = false;

void MouseCallBack(int event, int x, int y, int flags, void* userdata)
{
    if  ( event == EVENT_LBUTTONDOWN )
    {
        if(x < 640)
        {
            X1 = x;
            Y1 = y;
        }
        else if(x > 640)
        {
            X2 = x-640;
            Y2 = y;
        }
    }
    
}

int main()
{
    sp_port *thePort;
    sp_get_port_by_name("/dev/tty.usbserial-DA005RR2", &thePort);
    int ret = sp_open(thePort, SP_MODE_WRITE);
    if(ret == SP_OK)
    {
        printf("Serial port opened!\n");
        sp_set_baudrate(thePort, 19200);
        sp_set_bits(thePort, 8);
        sp_set_parity(thePort, SP_PARITY_NONE);
        sp_set_stopbits(thePort, 1);
        sp_set_flowcontrol(thePort, SP_FLOWCONTROL_NONE);
    }
    else
    {
        printf("Cannot open serial port!\n");
//        return 0;
    }
    
    VideoCapture camera(2);
    camera.set(CV_CAP_PROP_FRAME_WIDTH, CAMERAWIDTH);
    camera.set(CV_CAP_PROP_FRAME_HEIGHT, CAMERAHEIGHT);
//    camera.set(CV_CAP_PROP_EXPOSURE, -4);
    
    
    VideoCapture camera2(1);
    camera2.set(CV_CAP_PROP_FRAME_WIDTH, CAMERAWIDTH);
    camera2.set(CV_CAP_PROP_FRAME_HEIGHT, CAMERAHEIGHT);
    camera2.set(CV_CAP_PROP_EXPOSURE, -4);
    namedWindow("frame", WINDOW_AUTOSIZE);
    setMouseCallback("frame", MouseCallBack, NULL);
    
    bool detect=false;
    double camx1=0.0, camx2=0.0, camy1=0.0, camy2=0.0;
    
    Mat result, prev_frame, current_frame, next_frame;
    camera.read(current_frame);
    camera.read(prev_frame);
    camera.read(result);
    camera.read(next_frame);

    Mat result2, prev_frame2, current_frame2, next_frame2;
    camera2.read(current_frame2);
    camera2.read(next_frame2);
    camera2.read(prev_frame2);
    camera2.read(result2);

    cvtColor(current_frame, current_frame, COLOR_RGB2GRAY);
    cvtColor(prev_frame, prev_frame, COLOR_RGB2GRAY);
    cvtColor(next_frame, next_frame, COLOR_RGB2GRAY);
    cvtColor(current_frame2, current_frame2, COLOR_RGB2GRAY);
    cvtColor(prev_frame2, prev_frame2, COLOR_RGB2GRAY);
    cvtColor(next_frame2, next_frame2, COLOR_RGB2GRAY);

    Mat referenceFrame = current_frame;
    Mat d1, d2, motion;
    Mat d12, d22, motion2;

    Scalar mean_, color(0,255,255); // yellow
    // Detect motion in window
    int x_start = 0, x_stop = CAMERAWIDTH;
    int y_start = 50, y_stop = CAMERAHEIGHT;
    int x_start2 = 50, x_stop2 = 550;
    int y_start2 = 0, y_stop2 = CAMERAHEIGHT;
    // Maximum deviation of the image, the higher the value, the more motion is allowed
    int max_deviation = MAXMOTION;
    // Erode kernel
    Mat kernel_ero = getStructuringElement(MORPH_RECT, Size(3,2));
    vector<pair<int,int> > pointStorage;
    vector<pair<int,int> > pointStorage2;

    
    bool gotThreePoints = false;
    bool gotTwoPoints = false;

    while(true)
    {
        // Take a new image
        prev_frame = current_frame;
        current_frame = next_frame;
        prev_frame2 = current_frame2;
        current_frame2 = next_frame2;
        camera.read(next_frame);
        camera2.read(next_frame2);

        result = next_frame;
        cvtColor(next_frame, next_frame, COLOR_RGB2GRAY);

        result2 = next_frame2;
        cvtColor(next_frame2, next_frame2, COLOR_RGB2GRAY);

        absdiff(prev_frame, next_frame, d1);
        absdiff(next_frame, current_frame, d2);
        bitwise_and(d1, d2, motion);
        threshold(motion, motion, 20, 255, THRESH_BINARY);

        erode(motion, motion, kernel_ero);
        absdiff(prev_frame2, next_frame2, d12);
        absdiff(next_frame2, current_frame2, d22);
        bitwise_and(d12, d22, motion2);
        threshold(motion2, motion2, 20, 255, THRESH_BINARY);

        erode(motion2, motion2, kernel_ero);
        pair<int, int> pointTemp = detectMotion(motion, result, x_start, x_stop, y_start, y_stop, max_deviation, color);
        pair<int, int> pointTemp2 = detectMotion(motion2, result2, x_start2, x_stop2, y_start2, y_stop2, max_deviation, color);
        if(pointTemp.first !=0 && pointTemp.second !=0 /*&& (pointStorage.size()==0|| (abs(pointTemp.first - pointStorage[pointStorage.size()-1].first) < 50 && abs(pointTemp.second - pointStorage[pointStorage.size()-1].second) < 50))*/)
        {
            pointStorage.push_back(pointTemp);
        }
        
        if(pointTemp2.first !=0 && pointTemp2.second !=0 /*&& (pointStorage2.size() == 0 || (abs(pointTemp2.first - pointStorage2[pointStorage2.size()-1].first) < 50 && abs(pointTemp2.second - pointStorage2[pointStorage2.size()-1].second) < 50))*/)
        {
            pointStorage2.push_back(pointTemp2);
        }
        
        for(unsigned int i = 0; i < pointStorage.size();i++)
        {
            if(i > 0)
            {
                line(result, Point(pointStorage[i].first,pointStorage[i].second),Point(pointStorage[i-1].first,pointStorage[i-1].second), Scalar(0,0,255,1), 2);
            }
            circle(result, Point(pointStorage[i].first,pointStorage[i].second), 3, Scalar(255,0,0,1), 2);
        }
        for(unsigned int i = 0; i < pointStorage2.size();i++)
        {
            if(i > 0)
            {
                line(result2, Point(pointStorage2[i].first,pointStorage2[i].second),Point(pointStorage2[i-1].first,pointStorage2[i-1].second), Scalar(0,0,255,1), 2);
            }
            circle(result2, Point(pointStorage2[i].first,pointStorage2[i].second), 3, Scalar(255,0,0,1), 2);
        }
        if(pointStorage2.size() == 7)
        {
            gotTwoPoints = true;
        }
        if(gotTwoPoints)
        {
            float x0 = pointStorage2[1].first;
            float y0 = pointStorage2[1].second;
            float x1 = x0;
            float y1 = y0;
            float dx = 0.0f;
            if(pointStorage2[1].first < pointStorage2[0].first)
                dx = -20.0f;
            else
            {
                dx = 20.0f;
                //A = -A;
            }
            double pointStorage2ComponentX[6]={static_cast<double>(pointStorage2[1].first),static_cast<double>(pointStorage2[2].first),static_cast<double>(pointStorage2[3].first),static_cast<double>(pointStorage2[4].first),static_cast<double>(pointStorage2[5].first),static_cast<double>(pointStorage2[6].first)};
            double pointStorage2ComponentY[6]={static_cast<double>(pointStorage2[1].second),static_cast<double>(pointStorage2[2].second),static_cast<double>(pointStorage2[3].second),static_cast<double>(pointStorage2[4].second),static_cast<double>(pointStorage2[5].second),static_cast<double>(pointStorage2[6].second)};
            double D=0, E=0;

            llsq(6, pointStorage2ComponentX, pointStorage2ComponentY,D,E);

            //Principle line in second camera
            double D1=0,E1=0;
            ComputeLine(242,364,239,150,D1,E1);
            LineLine(D,E,D1,E1,camx2,camy2);

            for(int j = 0; j < 40; j++)
            {
                float x2 = x0 + dx * j;
                float y2 = y0 + j * dx * D;
                circle(result2, Point(x2,y2), 3, Scalar(0,255,255,1),2);
                line(result2, Point(x1,y1), Point(x2, y2), Scalar(0,255,0,1), 2);
                x1 = x2;
                y1 = y2;
            }
        }
        if(pointStorage.size() == 7)
        {
            gotThreePoints = true;
        }
        if(gotThreePoints)
        {
            double A=0.0,B = 0.0,C = 0.0;
            GetParabola(A, B, C, pointStorage);
            
            float x0 = pointStorage[0].first;
            float y0 = pointStorage[0].second;
            float x1 = x0;
            float y1 = y0;
            float dx = 0.0f;
            if(pointStorage[2].first < pointStorage[0].first)
                dx = -20.0f;
            else
            {
                dx = 20.0f;
            }
            
            if(camy2<365&&camy2>=311){camx1=(530-511)*(camy2-365)/(311-365)+511;}
            else if(camy2<311&&camy2>=255){camx1=(552-530)*(camy2-311)/(255-311)+530;}
            else if(camy2<255&&camy2>=203){camx1=(581-552)*(camy2-255)/(203-255)+552;}
            else if(camy2<203&&camy2>=150){camx1=(615-581)*(camy2-203)/(150-203)+581;}
            else camx1=0;
            
            
            camy1= A*pow(camx1,2) + B*camx1 + C;
            for(int j = 0; j < 40; j++)
            {
                float x2 = x0 + dx * j;
                float y2 = A*pow(x2,2) + B*x2 + C;
                circle(result, Point(x2,y2), 3, Scalar(0,255,255,1),2);
                line(result, Point(x1,y1), Point(x2, y2), Scalar(0,255,0,1), 2);
                x1 = x2;
                y1 = y2;
            }
        }

        circle(result, Point(int(camx1),int(camy1)), 7, Scalar(0,255,122,1),2);
        circle(result2, Point(int(camx2),int(camy2)), 7, Scalar(0,255,122,1),2);
        
        circle(result, Point(X1,Y1), 3, Scalar(0,0,255), 3);
        circle(result2, Point(X2,Y2), 3, Scalar(0,0,255), 3);
//        cout<<"Camera Pixel:"<<'('<<int(camx1)<<','<<int(camy1)<<"),("<<int(camx2)<<','<<int(camy2)<<')' <<endl;
        Point3DD realWorldResult = GetRealWorldCoordinates(int(camx1), int(camy1), int(camx2), int(camy2));
        //Point3DD realWorldResult = GetRealWorldCoordinates(X1, Y1, X2, Y2);
        if(realWorldResult.x!=0  && realWorldResult.z!=0)
        {
            if(!detect)
            {
                //printf("%i,%i | %i,%i\n", X1,Y1,X2,Y2);

            	printf("Found real world coordinate: %f %f %f\n", realWorldResult.x, realWorldResult.y, realWorldResult.z);
                ArmAngles res = GetArmAngles(Point3DD(realWorldResult.y/100.0, realWorldResult.x/100.0, realWorldResult.z/100.0));
                if(res.valid)
                {
                    printf("Found arm angles: %i %i %i %i %i\n", res.base - 900, res.shoulder, res.elbow, res.wristA, res.wristB);
                    moveROCA(thePort, res.base-900, res.shoulder, res.elbow, res.wristA, res.wristB);
                }
                else
                    printf("Angles invalid!\n");
                detect = true;
            }
        }
        Mat merged_frame = Mat(Size(1280, 480), CV_8UC3);
        Mat roi = Mat(merged_frame, Rect(0, 0, 640, 480));
        result.copyTo(roi);
        roi = Mat(merged_frame, Rect(640, 0, 640, 480));
        result2.copyTo(roi);
        imshow("frame", merged_frame);
        
        int key = waitKey(1);
        
        if(key == 27)
        {
            break;
        }
        else if(key == 97)
        {
            detect = false;
            pointStorage.clear();
            pointStorage2.clear();
            gotTwoPoints = false;
            gotThreePoints = false;
        }
    }

    sp_close(thePort);
    return 0;
}
