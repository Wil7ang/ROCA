#include <string>
#include <stdio.h>
#include "Prediction.h"

using namespace std;

Point3DD GetRealWorldCoordinates(int X1, int Y1, int X2, int Y2)
{
    double realX = 0.0;
    double realY = 20.0;
    double realZ = 0.0;
    // 0, 0 to 1, 1
    if(X1 > 511 && X1 <= 530 && Y1 > 269 && Y1 <= 299 && Y2 <= 361 && Y2 > 306)
    {
        realX = ((361.0 - (double)Y2)/55.0 * 35.0 + 0.0) - 70.0;
        double adjustedY1 = (double)Y1 - (361.0 - (double)Y2)/(217.0) * 14.0;
        realZ = (((297 - adjustedY1)/28) * 35.0);
        return Point3DD(realX,realY,realZ);
    }
    
    
    // 1, 0 to 2, 1
    if(X1 > 530 && X1 <= 552 && Y1 > 269 && Y1 <= 301 && Y2 <= 306 && Y2 > 251)
    {
        realX = ((306.0 - (double)Y2)/55.0 * 35.0 + 35.0) - 70.0;
        double adjustedY1 = (double)Y1 - (361.0 - (double)Y2)/(217.0) * 14.0;
        realZ = (((297 - adjustedY1)/28) * 35.0);
        return Point3DD(realX,realY,realZ);
    }
    
    
    // 2, 0 to 3, 1
    if(X1 > 552 && X1 <= 582 && Y1 > 270 && Y1 <= 307 && Y2 <= 251 && Y2 > 200)
    {
        realX = ((251.0 - (double)Y2)/51.0 * 35.0 + 70.0) - 70.0;
        double adjustedY1 = (double)Y1 - (361.0 - (double)Y2)/(217.0) * 14.0;
        realZ = (((297 - adjustedY1)/28) * 35.0);
        return Point3DD(realX,realY,realZ);
    }
    
    
    // 3, 0 to 4, 1
    if(X1 > 582 && X1 <= 616 && Y1 > 270 && Y1 <= 311 && Y2 <= 200 && Y2 > 144)
    {
        realX = ((200.0 - (double)Y2)/56.0 * 35.0 + 105.0) - 70.0;
        double adjustedY1 = (double)Y1 - (361.0 - (double)Y2)/(217.0) * 14.0;
        realZ = (((297 - adjustedY1)/28) * 35.0);
        return Point3DD(realX,realY,realZ);
    }
    
    
    // 0, 1 to 1, 2
    if(X1 > 512 && X1 <= 530 && Y1 > 241 && Y1 <= 269 && Y2 <= 363 && Y2 > 307)
    {
        realX = ((363.0 - (double)Y2)/56.0 * 35.0 + 0.0) - 70.0;
        double adjustedY1 = (double)Y1 - (363.0 - (double)Y2)/(217.0) * 1.0;
        realZ = (((297 - adjustedY1)/28) * 35.0);
        return Point3DD(realX,realY,realZ);
    }
    
    
    // 1, 1 to 2, 2
    if(X1 > 530 && X1 <= 553 && Y1 > 238 && Y1 <= 270 && Y2 <= 307 && Y2 > 252)
    {
        realX = ((307.0 - (double)Y2)/55.0 * 35.0 + 35.0) - 70.0;
        double adjustedY1 = (double)Y1 - (363.0 - (double)Y2)/(217.0) * 1.0;
        realZ = (((297 - adjustedY1)/28) * 35.0);
        return Point3DD(realX,realY,realZ);
    }
    
    
    // 2, 1 to 3, 2
    if(X1 > 553 && X1 <= 581 && Y1 > 235 && Y1 <= 270 && Y2 <= 252 && Y2 > 197)
    {
        realX = ((252.0 - (double)Y2)/55.0 * 35.0 + 70.0) - 70.0;
        double adjustedY1 = (double)Y1 - (363.0 - (double)Y2)/(217.0) * 1.0;
        realZ = (((297 - adjustedY1)/28) * 35.0);
        return Point3DD(realX,realY,realZ);
    }
    
    
    // 3, 1 to 4, 2
    if(X1 > 581 && X1 <= 615 && Y1 > 231 && Y1 <= 270 && Y2 <= 197 && Y2 > 146)
    {
        realX = ((197.0 - (double)Y2)/51.0 * 35.0 + 105.0) - 70.0;
        double adjustedY1 = (double)Y1 - (363.0 - (double)Y2)/(217.0) * 1.0;
        realZ = (((297 - adjustedY1)/28) * 35.0);
        return Point3DD(realX,realY,realZ);
    }
    
    return Point3DD(0,0,0);
}