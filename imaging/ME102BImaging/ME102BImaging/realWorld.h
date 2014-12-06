#include <string>
#include <stdio.h>
#include "Prediction.h"

using namespace std;

Point3DD GetRealWorldCoordinates(int X1, int Y1, int X2, int Y2)
{
    double realX = 0.0;
    double realY = 35.0;
    double realZ = 0.0;
    // 0, 0 to 1, 1
    if(X1 > 510 && X1 <= 531 && Y1 > 293 && Y1 <= 324 && Y2 <= 351 && Y2 > 295)
    {
        realX = ((351.0 - (double)Y2)/56.0 * 35.0 + 0.0) - 70.0;
        double adjustedY1 = (double)Y1 - (351.0 - (double)Y2)/(231.0) * 12.0;
        realZ = (((321 - adjustedY1)/28) * 35.0);
        return Point3DD(realX,realY,realZ);
    }
    
    
    // 1, 0 to 2, 1
    if(X1 > 531 && X1 <= 552 && Y1 > 292 && Y1 <= 327 && Y2 <= 295 && Y2 > 236)
    {
        realX = ((295.0 - (double)Y2)/59.0 * 35.0 + 35.0) - 70.0;
        double adjustedY1 = (double)Y1 - (351.0 - (double)Y2)/(231.0) * 12.0;
        realZ = (((321 - adjustedY1)/28) * 35.0);
        return Point3DD(realX,realY,realZ);
    }
    
    
    // 2, 0 to 3, 1
    if(X1 > 552 && X1 <= 578 && Y1 > 291 && Y1 <= 328 && Y2 <= 236 && Y2 > 181)
    {
        realX = ((236.0 - (double)Y2)/55.0 * 35.0 + 70.0) - 70.0;
        double adjustedY1 = (double)Y1 - (351.0 - (double)Y2)/(231.0) * 12.0;
        realZ = (((321 - adjustedY1)/28) * 35.0);
        return Point3DD(realX,realY,realZ);
    }
    
    
    // 3, 0 to 4, 1
    if(X1 > 578 && X1 <= 611 && Y1 > 293 && Y1 <= 333 && Y2 <= 181 && Y2 > 120)
    {
        realX = ((181.0 - (double)Y2)/61.0 * 35.0 + 105.0) - 70.0;
        double adjustedY1 = (double)Y1 - (351.0 - (double)Y2)/(231.0) * 12.0;
        realZ = (((321 - adjustedY1)/28) * 35.0);
        return Point3DD(realX,realY,realZ);
    }
    
    
    // 0, 1 to 1, 2
    if(X1 > 511 && X1 <= 526 && Y1 > 265 && Y1 <= 292 && Y2 <= 351 && Y2 > 294)
    {
        realX = ((351.0 - (double)Y2)/57.0 * 35.0 + 0.0) - 70.0;
        double adjustedY1 = (double)Y1 - (351.0 - (double)Y2)/(231.0) * -3.0;
        realZ = (((321 - adjustedY1)/28) * 35.0);
        return Point3DD(realX,realY,realZ);
    }
    
    
    // 1, 1 to 2, 2
    if(X1 > 526 && X1 <= 549 && Y1 > 262 && Y1 <= 291 && Y2 <= 294 && Y2 > 237)
    {
        realX = ((294.0 - (double)Y2)/57.0 * 35.0 + 35.0) - 70.0;
        double adjustedY1 = (double)Y1 - (351.0 - (double)Y2)/(231.0) * -3.0;
        realZ = (((321 - adjustedY1)/28) * 35.0);
        return Point3DD(realX,realY,realZ);
    }
    
    
    // 2, 1 to 3, 2
    if(X1 > 549 && X1 <= 576 && Y1 > 258 && Y1 <= 293 && Y2 <= 237 && Y2 > 180)
    {
        realX = ((237.0 - (double)Y2)/57.0 * 35.0 + 70.0) - 70.0;
        double adjustedY1 = (double)Y1 - (351.0 - (double)Y2)/(231.0) * -3.0;
        realZ = (((321 - adjustedY1)/28) * 35.0);
        return Point3DD(realX,realY,realZ);
    }
    
    
    // 3, 1 to 4, 2
    if(X1 > 576 && X1 <= 606 && Y1 > 252 && Y1 <= 290 && Y2 <= 180 && Y2 > 120)
    {
        realX = ((180.0 - (double)Y2)/60.0 * 35.0 + 105.0) - 70.0;
        double adjustedY1 = (double)Y1 - (351.0 - (double)Y2)/(231.0) * -3.0;
        realZ = (((321 - adjustedY1)/28) * 35.0);
        return Point3DD(realX,realY,realZ);
    }
    
    
    return Point3DD(0,0,0);
}