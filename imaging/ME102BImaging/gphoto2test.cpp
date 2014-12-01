#include <stdio.h>
#include <stdlib.h>
#include <cstdlib>
#include <unistd.h>

#include "opencv2/opencv.hpp"
#include "opencv2/highgui/highgui.hpp"

#include <vector>
#include <time.h>
#include <cmath>

#include "realWorld.h"

#define CAMERAWIDTH 640
#define CAMERAHEIGHT 480
#define MAXMOTION 100


#include "prediction.h"

using namespace cv;
using namespace std;

#include <unistd.h>

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

float determinant3x3(float a, float b, float c,
                     float d, float e, float f,
                     float g, float h, float i)
{
    return a*e*i + b*f*g + c*d*h - a*f*h - b*d*i - c*e*g;
}

int main()
{
//    CvCapture * camera = cvCaptureFromCAM(2);
//    cvSetCaptureProperty(camera, CV_CAP_PROP_FRAME_WIDTH, CAMERAWIDTH); // width of viewport of camera
//    cvSetCaptureProperty(camera, CV_CAP_PROP_FRAME_HEIGHT, CAMERAHEIGHT); // height of ...
//
//    CvCapture * camera2 = cvCaptureFromCAM(1);
//    cvSetCaptureProperty(camera2, CV_CAP_PROP_FRAME_WIDTH, CAMERAWIDTH); // width of viewport of camera
//    cvSetCaptureProperty(camera2, CV_CAP_PROP_FRAME_HEIGHT, CAMERAHEIGHT); // height of ...

    VideoCapture camera(1);
    camera.set(CV_CAP_PROP_EXPOSURE, -6);
    VideoCapture camera2(2);
    camera2.set(CV_CAP_PROP_EXPOSURE, -5);

    namedWindow("result", WINDOW_AUTOSIZE);
    namedWindow("result2", WINDOW_AUTOSIZE);

    Mat result;
    Mat prev_frame;
    camera.read(prev_frame);
    camera.read(result);
    Mat current_frame;
    camera.read(current_frame);
    Mat next_frame;
    camera.read(next_frame);

    Mat result2;
    Mat prev_frame2;
    camera2.read(prev_frame2);
    camera2.read(result2);
    Mat current_frame2;
    camera2.read(current_frame2);
    Mat next_frame2;
    camera2.read(next_frame2);



    cvtColor(current_frame, current_frame, CV_RGB2GRAY);
    cvtColor(prev_frame, prev_frame, CV_RGB2GRAY);
    cvtColor(next_frame, next_frame, CV_RGB2GRAY);

    cvtColor(current_frame2, current_frame2, CV_RGB2GRAY);
    cvtColor(prev_frame2, prev_frame2, CV_RGB2GRAY);
    cvtColor(next_frame2, next_frame2, CV_RGB2GRAY);

    Mat referenceFrame = current_frame;

    Mat d1, d2, motion;
    Mat d12, d22, motion2;
    int number_of_changes, number_of_sequence = 0;
    Scalar mean_, color(0,255,255); // yellow

    // Detect motion in window
    int x_start = 0, x_stop = CAMERAWIDTH;
    int y_start = 0, y_stop = CAMERAHEIGHT;
    int x_start2 = 0, x_stop2 = CAMERAWIDTH;
    int y_start2 = 0, y_stop2 = CAMERAHEIGHT;


    // Maximum deviation of the image, the higher the value, the more motion is allowed
    int max_deviation = MAXMOTION;

    // Erode kernel
    Mat kernel_ero = getStructuringElement(MORPH_RECT, Size(3,3));
    vector<pair<int,int> > pointStorage;
    vector<pair<int,int> > pointStorage2;

    //    clock_t curTime = 0;
    //    bool gotTwoPoints = false;
    bool gotThreePoints = false;
    bool gotTwoPoints = false;

    //    float dt = 0.0f;
    int counter = 0;

    double collecttime[4];
    int cycle = -1;

    bool velocityCalculated = false;

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
        cvtColor(next_frame, next_frame, CV_RGB2GRAY);

        result2 = next_frame2;
        cvtColor(next_frame2, next_frame2, CV_RGB2GRAY);

        // Calc differences between the images and do AND-operation
        // threshold image, low differences are ignored (ex. contrast change due to sunlight)
        absdiff(prev_frame, next_frame, d1);
        absdiff(next_frame, current_frame, d2);
        bitwise_and(d1, d2, motion);
        threshold(motion, motion, 20, 255, CV_THRESH_BINARY);
        erode(motion, motion, kernel_ero);

        absdiff(prev_frame2, next_frame2, d12);
        absdiff(next_frame2, current_frame2, d22);
        bitwise_and(d12, d22, motion2);
        threshold(motion2, motion2, 20, 255, CV_THRESH_BINARY);
        erode(motion2, motion2, kernel_ero);


        pair<int, int> pointTemp = detectMotion(motion, result,  x_start, x_stop, y_start, y_stop, max_deviation, color);
        pair<int, int> pointTemp2 = detectMotion(motion2, result2,  x_start2, x_stop2, y_start2, y_stop2, max_deviation, color);

        if(pointTemp2.first !=0 && pointTemp2.second !=0 && pointTemp.first !=0 && pointTemp.second !=0)
        {
            if(cycle < 0)
            {
                cycle++;
                collecttime[cycle] = (double)getTickCount();
            pointStorage2.push_back(pointTemp2);
            pointStorage.push_back(pointTemp);
            }
            else if(cycle < 3)
            {
                if(((double)getTickCount() - collecttime[cycle])/getTickFrequency() > 0.03)
                {
                    cycle++;
                    collecttime[cycle] = (double)getTickCount();
                    pointStorage2.push_back(pointTemp2);
                    pointStorage.push_back(pointTemp);
                }
            }
            else if(cycle >=3)
            {
                pointStorage2.push_back(pointTemp2);
                    pointStorage.push_back(pointTemp);
            }
        }

        for(unsigned int i = 0; i < pointStorage.size();i++)
        {
            if(i > 0)
            {
                line(result, Point(pointStorage[i].first,pointStorage[i].second),Point(pointStorage[i-1].first,pointStorage[i-1].second), Scalar(0,0,255,1), 2);
            }
            circle(result, Point(pointStorage[i].first,pointStorage[i].second), 3, Scalar(255,0,0,1), 2);
            double realX=0.0,realY=0.0,realZ=0.0;
            GetRealWorldCoordinates(realX,realY,realZ,pointStorage[i].first,pointStorage[i].second,pointStorage2[i].first,pointStorage2[i].second);
            if(realX != 0.0 && realY != 0.0 && realZ != 0.0)
            putText(result,to_string(realZ),Point(pointStorage[i].first,pointStorage[i].second+30),1,1,Scalar(0,255,0),2);
        }


        for(unsigned int i = 0; i < pointStorage2.size();i++)
        {

            if(i > 0)
            {
                line(result2, Point(pointStorage2[i].first,pointStorage2[i].second),Point(pointStorage2[i-1].first,pointStorage2[i-1].second), Scalar(0,0,255,1), 2);
            }
            circle(result2, Point(pointStorage2[i].first,pointStorage2[i].second), 3, Scalar(255,0,0,1), 2);
        }

        if(pointStorage2.size() == 6)
        {
            gotTwoPoints = true;
        }

//        if(gotTwoPoints)
//        {
//
//                double x0, y0, z0;
//                double x1, y1, z1;
//                double x2, y2, z2;
//                double x3, y3, z3;
//                GetRealWorldCoordinates(x0, y0, z0, pointStorage[0].first, pointStorage[0].second, pointStorage2[0].first, pointStorage2[0].second);
//                GetRealWorldCoordinates(x1, y1, z1, pointStorage[1].first, pointStorage[1].second, pointStorage2[1].first, pointStorage2[1].second);
//                GetRealWorldCoordinates(x2, y2, z2, pointStorage[2].first, pointStorage[2].second, pointStorage2[2].first, pointStorage2[2].second);
//                GetRealWorldCoordinates(x3, y3, z3, pointStorage[3].first, pointStorage[3].second, pointStorage2[3].first, pointStorage2[3].second);
//
//
//
//
//                x0 = 0.0254*x0;
//                x1 = 0.0254*x1;
//                x2 = 0.0254*x2;
//                x3 = 0.0254*x3;
//
//
//                y0 = 5-0.0254*y0;
//                y1 = 5-0.0254*y1;
//                y2 = 5-0.0254*y2;
//                y3 = 5-0.0254*y3;
//
//                z0 = 0.0254*(z0+19.25);
//                z1 = 0.0254*(z1+19.25);
//                z2 = 0.0254*(z2+19.25);
//                z3 = 0.0254*(z3+19.25);
//
//                double v_x0 = 0.7*((x1-x0)/((collecttime[1]-collecttime[0])/getTickFrequency()) +
//                              (x2-x1)/((collecttime[2]-collecttime[1])/getTickFrequency()) +
//                              (x3-x2)/((collecttime[3]-collecttime[2])/getTickFrequency()))/3.0;
//
//                double v_y0 = 0.7*((y1-y0)/((collecttime[1]-collecttime[0])/getTickFrequency()) +
//                              (y2-y1)/((collecttime[2]-collecttime[1])/getTickFrequency()) +
//                              (y3-y2)/((collecttime[3]-collecttime[2])/getTickFrequency()))/3.0;
//
//                double v_z0 = 0.7*((z1-z0)/((collecttime[1]-collecttime[0])/getTickFrequency()) +
//                              (z2-z1)/((collecttime[2]-collecttime[1])/getTickFrequency()) +
//                              (z3-z2)/((collecttime[3]-collecttime[2])/getTickFrequency()))/3.0;
//
//                printf("%f %f %f %f\n", v_x0, v_y0, v_z0, ((collecttime[1]-collecttime[0])/getTickFrequency()));
//
//                Point3DD result = FindCatchLocation(v_y0, v_x0, v_z0, y1, x1, z1-37.0*0.0254);
//                printf("%f %f %f\n", result.x, result.y, result.z);
//                ArmAngles theAngles = GetArmAngles(result);
//                printf("%i %i %i\n", theAngles.base, theAngles.shoulder, theAngles.elbow);

//        }


        if(gotTwoPoints)
        {
            float x0 = pointStorage2[3].first;
            float y0 = pointStorage2[3].second;

            float x1 = x0;
            float y1 = y0;

            float dx = 0.0f;
            if(pointStorage2[3].first < pointStorage2[2].first)
                dx = -20.0f;
            else
            {
                dx = 20.0f;
                //A = -A;
            }
            for(int j = 0; j < 40; j++)
            {
                float x2 = x0 + dx * j;
                float y2 = y0 + j * dx * (pointStorage2[5].second - pointStorage2[2].second)/(pointStorage2[5].first - pointStorage2[2].first);
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
            float detA = determinant3x3(pointStorage[0].second, pointStorage[0].first, 1,
                                        pointStorage[3].second, pointStorage[3].first, 1,
                                        pointStorage[6].second, pointStorage[6].first, 1);

            float detB = determinant3x3(pow((float)pointStorage[0].first,2), pointStorage[0].second, 1,
                                        pow((float)pointStorage[3].first,2), pointStorage[3].second, 1,
                                        pow((float)pointStorage[6].first,2), pointStorage[6].second, 1);

            float detC = determinant3x3(pow((float)pointStorage[0].first,2), pointStorage[0].first, pointStorage[0].second,
                                        pow((float)pointStorage[3].first,2), pointStorage[3].first, pointStorage[3].second,
                                        pow((float)pointStorage[6].first,2), pointStorage[6].first, pointStorage[6].second);

            float detD = determinant3x3(pow((float)pointStorage[0].first,2), pointStorage[0].first, 1,
                                        pow((float)pointStorage[3].first,2), pointStorage[3].first, 1,
                                        pow((float)pointStorage[6].first,2), pointStorage[6].first, 1);

            float A = detA/detD;
            float B = detB/detD;
            float C = detC/detD;

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
                //A = -A;
            }
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

        imshow("result",result);
        imshow("result2",result2);

        int key = waitKey(1);
        if(key == 27)
        {
            break;
        }
        else if(key == 97)
        {
            pointStorage.clear();
            pointStorage2.clear();
            gotTwoPoints = false;
            gotThreePoints = false;

            cycle = -1;
        }
    }

    return 0;
}
