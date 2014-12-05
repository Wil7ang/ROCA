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
    if(X1 > 509 && X1 <= 528 && Y1 > 285 && Y1 <= 314 && Y2 <= 342 && Y2 > 285)
    {
        realX = ((342.0 - (double)Y2)/57.0 * 35.0 + 0.0) - 70.0;
        double adjustedY1 = (double)Y1 - (342.0 - (double)Y2)/(234.0) * 11.0;
        realZ = (((313 - adjustedY1)/28) * 35.0);
        return Point3DD(realX,realY,realZ);
    }
    
    
    // 1, 0 to 2, 1
    if(X1 > 528 && X1 <= 548 && Y1 > 285 && Y1 <= 315 && Y2 <= 285 && Y2 > 224)
    {
        realX = ((285.0 - (double)Y2)/61.0 * 35.0 + 35.0) - 70.0;
        double adjustedY1 = (double)Y1 - (342.0 - (double)Y2)/(234.0) * 11.0;
        realZ = (((313 - adjustedY1)/28) * 35.0);
        return Point3DD(realX,realY,realZ);
    }
    
    
    // 2, 0 to 3, 1
    if(X1 > 548 && X1 <= 580 && Y1 > 286 && Y1 <= 318 && Y2 <= 224 && Y2 > 168)
    {
        realX = ((224.0 - (double)Y2)/56.0 * 35.0 + 70.0) - 70.0;
        double adjustedY1 = (double)Y1 - (342.0 - (double)Y2)/(234.0) * 11.0;
        realZ = (((313 - adjustedY1)/28) * 35.0);
        return Point3DD(realX,realY,realZ);
    }
    
    
    // 3, 0 to 4, 1
    if(X1 > 580 && X1 <= 613 && Y1 > 285 && Y1 <= 324 && Y2 <= 168 && Y2 > 108)
    {
        realX = ((168.0 - (double)Y2)/60.0 * 35.0 + 105.0) - 70.0;
        double adjustedY1 = (double)Y1 - (342.0 - (double)Y2)/(234.0) * 11.0;
        realZ = (((313 - adjustedY1)/28) * 35.0);
        return Point3DD(realX,realY,realZ);
    }
    
    
    // 0, 1 to 1, 2
    if(X1 > 509 && X1 <= 528 && Y1 > 258 && Y1 <= 285 && Y2 <= 341 && Y2 > 284)
    {
        realX = ((341.0 - (double)Y2)/57.0 * 35.0 + 0.0) - 70.0;
        double adjustedY1 = (double)Y1 - (341.0 - (double)Y2)/(237.0) * -1.0;
        realZ = (((313 - adjustedY1)/28) * 35.0);
        return Point3DD(realX,realY,realZ);
    }
    
    
    // 1, 1 to 2, 2
    if(X1 > 528 && X1 <= 549 && Y1 > 253 && Y1 <= 286 && Y2 <= 284 && Y2 > 225)
    {
        realX = ((284.0 - (double)Y2)/59.0 * 35.0 + 35.0) - 70.0;
        double adjustedY1 = (double)Y1 - (341.0 - (double)Y2)/(237.0) * -1.0;
        realZ = (((313 - adjustedY1)/28) * 35.0);
        return Point3DD(realX,realY,realZ);
    }
    
    
    // 2, 1 to 3, 2
    if(X1 > 549 && X1 <= 578 && Y1 > 250 && Y1 <= 285 && Y2 <= 225 && Y2 > 164)
    {
        realX = ((225.0 - (double)Y2)/61.0 * 35.0 + 70.0) - 70.0;
        double adjustedY1 = (double)Y1 - (341.0 - (double)Y2)/(237.0) * -1.0;
        realZ = (((313 - adjustedY1)/28) * 35.0);
        return Point3DD(realX,realY,realZ);
    }
    
    
    // 3, 1 to 4, 2
    if(X1 > 578 && X1 <= 610 && Y1 > 246 && Y1 <= 284 && Y2 <= 164 && Y2 > 104)
    {
        realX = ((164.0 - (double)Y2)/60.0 * 35.0 + 105.0) - 70.0;
        double adjustedY1 = (double)Y1 - (341.0 - (double)Y2)/(237.0) * -1.0;
        realZ = (((313 - adjustedY1)/28) * 35.0);
        return Point3DD(realX,realY,realZ);
    }
    
    return Point3DD(0,0,0);
}