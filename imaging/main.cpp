#ifdef CANON
#include <gphoto2/gphoto2.h>
#include <gphoto2/gphoto2-camera.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <cstdlib>
#include <unistd.h>

#include "opencv2/opencv.hpp"
#include "opencv2/highgui/highgui.hpp"

#include <vector>
#include <time.h>
#include <cmath>

#define CAMERAWIDTH 640
#define CAMERAHEIGHT 480
#define MAXMOTION 80

//#define CANON

using namespace cv;
using namespace std;

#ifdef CANON
static void errordumper(GPLogLevel level, const char *domain, const char *str, void *data)
{
    printf("%s (data %p)", str, data);
}
#endif

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

#ifdef CANON
Mat GetCameraImage(Camera *cam, GPContext *context)
{
    CameraFile *file = NULL;
    gp_file_new(&file);
    gp_camera_capture_preview(cam, file, context);
    
    const char *data = NULL;
    unsigned long int siz;
    gp_file_get_data_and_size(file, &data, &siz);
    
    vector<char> vdat(data, data+siz);
    gp_file_unref(file);
    file = NULL;
    
    return imdecode(Mat(vdat), CV_LOAD_IMAGE_ANYCOLOR);
}
#endif

float determinant3x3(float a, float b, float c,
                     float d, float e, float f,
                     float g, float h, float i)
{
    return a*e*i + b*f*g + c*d*h - a*f*h - b*d*i - c*e*g;
}

int main()
{
#ifdef CANON
    Camera *canon;
    GPContext *canonContext = gp_context_new();
    gp_camera_new(&canon);
    //gp_log_add_func(GP_LOG_ERROR, errordumper, 0);
    int retval = gp_camera_init(canon, canonContext);
    
    if(retval != GP_OK)
    {
        printf("%i\n", retval);
        return -1;
    }
    
    
    CameraWidget *widget = NULL;
    CameraWidget *child = NULL;
    CameraWidgetType type;
    
    int onoff = 1;
    gp_camera_get_config(canon, &widget, canonContext);
    gp_widget_get_child_by_name(widget, "eosviewfinder", &child);
    gp_widget_get_type(child, &type);
    gp_widget_set_value(child, &onoff);
    gp_camera_set_config(canon, widget, canonContext);
    
    sleep(2);
    
    Mat result;
    Mat prev_frame = result = GetCameraImage(canon, canonContext);
    Mat current_frame = GetCameraImage(canon, canonContext);
    Mat next_frame = GetCameraImage(canon, canonContext);
#else
    CvCapture * camera = cvCaptureFromCAM(0);
    cvSetCaptureProperty(camera, CV_CAP_PROP_FRAME_WIDTH, CAMERAWIDTH); // width of viewport of camera
    cvSetCaptureProperty(camera, CV_CAP_PROP_FRAME_HEIGHT, CAMERAHEIGHT); // height of ...

    CvCapture * camera2 = cvCaptureFromCAM(2);
    cvSetCaptureProperty(camera2, CV_CAP_PROP_FRAME_WIDTH, CAMERAWIDTH); // width of viewport of camera
    cvSetCaptureProperty(camera2, CV_CAP_PROP_FRAME_HEIGHT, CAMERAHEIGHT); // height of ...
    
    namedWindow("result", WINDOW_AUTOSIZE);
    namedWindow("result2", WINDOW_AUTOSIZE);
    //namedWindow("motion", WINDOW_AUTOSIZE);
    //namedWindow("d1", WINDOW_AUTOSIZE);
    //namedWindow("d2", WINDOW_AUTOSIZE);
    
    Mat result;
    Mat prev_frame = result = cvQueryFrame(camera);
    Mat current_frame = cvQueryFrame(camera);
    Mat next_frame = cvQueryFrame(camera);
    
    Mat result2;
    Mat prev_frame2 = result2 = cvQueryFrame(camera2);
    Mat current_frame2 = cvQueryFrame(camera2);
    Mat next_frame2 = cvQueryFrame(camera2);
#endif
    
    
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
    
    
    // Maximum deviation of the image, the higher the value, the more motion is allowed
    int max_deviation = MAXMOTION;
    
    // Erode kernel
    Mat kernel_ero = getStructuringElement(MORPH_RECT, Size(4,4));
    vector<pair<int,int> > pointStorage;
    vector<pair<int,int> > pointStorage2;
    
    //    clock_t curTime = 0;
    //    bool gotTwoPoints = false;
    bool gotThreePoints = false;
    bool gotTwoPoints = false;
    //    float dt = 0.0f;
    while(true)
    {
        // Take a new image
        prev_frame = current_frame;
        current_frame = next_frame;
        
        prev_frame2 = current_frame2;
        current_frame2 = next_frame2;
#ifdef CANON
        next_frame = GetCameraImage(canon, canonContext);
#else
        next_frame = cvQueryFrame(camera);
        next_frame2 = cvQueryFrame(camera2);
#endif
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
        //threshold(d1, d1, 20, 255, CV_THRESH_BINARY);
        //erode(d1, d1, kernel_ero);
        
        pair<int, int> pointTemp = detectMotion(motion, result,  x_start, x_stop, y_start, y_stop, max_deviation, color);
        if(pointTemp.first !=0 && pointTemp.second !=0)
            pointStorage.push_back(pointTemp);
        for(unsigned int i = 0; i < pointStorage.size();i++)
        {   if(i > 0)
        {
            line(result, Point(pointStorage[i].first,pointStorage[i].second),Point(pointStorage[i-1].first,pointStorage[i-1].second), Scalar(0,0,255,1), 2);
        }
            circle(result, Point(pointStorage[i].first,pointStorage[i].second), 3, Scalar(255,0,0,1), 2);
        }

        pair<int, int> pointTemp2 = detectMotion(motion2, result2,  x_start, x_stop, y_start, y_stop, max_deviation, color);
        if(pointTemp2.first !=0 && pointTemp2.second !=0)
            pointStorage2.push_back(pointTemp2);
        for(unsigned int i = 0; i < pointStorage2.size();i++)
        {   if(i > 0)
        {
            line(result2, Point(pointStorage2[i].first,pointStorage2[i].second),Point(pointStorage2[i-1].first,pointStorage2[i-1].second), Scalar(0,0,255,1), 2);
        }
            circle(result2, Point(pointStorage2[i].first,pointStorage2[i].second), 3, Scalar(255,0,0,1), 2);
        }
        //        if(pointStorage.size() == 1 && curTime == 0)
        //        {
        //            curTime = clock();
        //        }
        //        if(pointStorage.size() == 2 && !gotTwoPoints)
        //        {
        //            dt = ((float)(clock()-curTime))/CLOCKS_PER_SEC;
        //            gotTwoPoints = true;
        //        }
        
        if(pointStorage2.size() == 2)
        {
            gotTwoPoints = true;
        }
        
        if(gotTwoPoints)
        {
            float x0 = pointStorage2[0].first;
            float y0 = pointStorage2[0].second;
            
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
            for(int j = 0; j < 40; j++)
            {
                float x2 = x0 + dx * j;
                float y2 = y0 + j * dx * (pointStorage2[1].second - pointStorage2[0].second)/(pointStorage2[1].first - pointStorage2[0].first);
                circle(result2, Point(x2,y2), 3, Scalar(0,255,255,1),2);
                line(result2, Point(x1,y1), Point(x2, y2), Scalar(0,255,0,1), 2);
                x1 = x2;
                y1 = y2;
            }
        }
        
        if(pointStorage.size() == 3)
        {
            gotThreePoints = true;
        }
        
        if(gotThreePoints)
        {
            float detA = determinant3x3(pointStorage[0].second, pointStorage[0].first, 1,
                                        pointStorage[1].second, pointStorage[1].first, 1,
                                        pointStorage[2].second, pointStorage[2].first, 1);
            
            float detB = determinant3x3(pow((float)pointStorage[0].first,2), pointStorage[0].second, 1,
                                        pow((float)pointStorage[1].first,2), pointStorage[1].second, 1,
                                        pow((float)pointStorage[2].first,2), pointStorage[2].second, 1);
            
            float detC = determinant3x3(pow((float)pointStorage[0].first,2), pointStorage[0].first, pointStorage[0].second,
                                        pow((float)pointStorage[1].first,2), pointStorage[1].first, pointStorage[1].second,
                                        pow((float)pointStorage[2].first,2), pointStorage[2].first, pointStorage[2].second);
            
            float detD = determinant3x3(pow((float)pointStorage[0].first,2), pointStorage[0].first, 1,
                                        pow((float)pointStorage[1].first,2), pointStorage[1].first, 1,
                                        pow((float)pointStorage[2].first,2), pointStorage[2].first, 1);
            
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
        
        //        if(gotTwoPoints)
        //        {
        //            float velocityY = (pointStorage[1].second - pointStorage[0].second)/dt;
        //            float velocityX = (pointStorage[1].first - pointStorage[0].first)/dt;
        //
        //            float x0 = pointStorage[1].first;
        //            float y0 = pointStorage[1].second;
        //
        //            float x1 = x0;
        //            float y1 = y0;
        //            //Calculate parabola here
        //            for(float j = 0.0f; j < 2.0f; j+=0.01)
        //            {
        //                float x2 = x0 + velocityX * j;
        //                float y2 = 0.5 * 11000 * pow(j,2) + velocityY * j + y0;
        //                circle(result, Point(x2, y2), 3, Scalar(0,255,255,1), 2);
        //                line(result, Point(x1, y1), Point(x2, y2), Scalar(0,255,0,1), 2);
        //                x1 = x2;
        //                y1 = y2;
        //            }
        //        }
        
        //imshow("motion",motion);
        //imshow("d1",d1);
        //imshow("d2",d2);

        imshow("result",result);
        imshow("result2",result2);
        if(waitKey(30) == 27)
        {
            break;
        }
        else if(waitKey(30) != -1)
        {
            pointStorage.clear();
            pointStorage2.clear();
            //            gotTwoPoints = false;
            gotThreePoints = false;
            gotTwoPoints = false;
            //            curTime = 0;
            //            dt = 0.0f;
        }
        //        else if(pointStorage.size()>=2 && pointStorage[pointStorage.size()-1].first < pointStorage[pointStorage.size()-2].first-200)
        //        {
        //            pair<int,int> tStorage = pointStorage[pointStorage.size()-1];
        //            pointStorage.clear();
        //            pointStorage.push_back(tStorage);
        //        }
    }
#ifdef CANON
    onoff = 0;
    gp_widget_set_value(child, &onoff);
    gp_camera_set_config(canon, widget, canonContext);
    
    gp_camera_exit(canon, canonContext);
#endif
    return 0;
}
