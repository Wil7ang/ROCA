#include <string>
#include <stdio.h>
using namespace std;
void GetRealWorldCoordinates(double &realX, double &realY, double &realZ, int X1, int Y1, int X2, int Y2)
{
    // 0, 0 to 1, 1
    if(X1 > 503 && X1 <= 522 && Y1 > 287 && Y1 <= 319 && Y2 <= 351 && Y2 > 297)
    {
        realX = ((351.0 - (double)Y2)/54.0 * 35.0 + 0.0) - 70.0;
        double adjustedY1 = (double)Y1 - (351.0 - (double)Y2)/(216.0) * 15.0;
        realZ = (((315 - adjustedY1)/28) * 35.0);
    }
    
    
    // 1, 0 to 2, 1
    if(X1 > 522 && X1 <= 545 && Y1 > 288 && Y1 <= 321 && Y2 <= 297 && Y2 > 242)
    {
        realX = ((297.0 - (double)Y2)/55.0 * 35.0 + 35.0) - 70.0;
        double adjustedY1 = (double)Y1 - (351.0 - (double)Y2)/(216.0) * 15.0;
        realZ = (((315 - adjustedY1)/28) * 35.0);
    }
    
    
    // 2, 0 to 3, 1
    if(X1 > 545 && X1 <= 575 && Y1 > 287 && Y1 <= 325 && Y2 <= 242 && Y2 > 193)
    {
        realX = ((242.0 - (double)Y2)/49.0 * 35.0 + 70.0) - 70.0;
        double adjustedY1 = (double)Y1 - (351.0 - (double)Y2)/(216.0) * 15.0;
        realZ = (((315 - adjustedY1)/28) * 35.0);
    }
    
    
    // 3, 0 to 4, 1
    if(X1 > 575 && X1 <= 609 && Y1 > 290 && Y1 <= 330 && Y2 <= 193 && Y2 > 135)
    {
        realX = ((193.0 - (double)Y2)/58.0 * 35.0 + 105.0) - 70.0;
        double adjustedY1 = (double)Y1 - (351.0 - (double)Y2)/(216.0) * 15.0;
        realZ = (((315 - adjustedY1)/28) * 35.0);
    }
    
    
    // 0, 1 to 1, 2
    if(X1 > 501 && X1 <= 520 && Y1 > 259 && Y1 <= 288 && Y2 <= 350 && Y2 > 296)
    {
        realX = ((350.0 - (double)Y2)/54.0 * 35.0 + 0.0) - 70.0;
        double adjustedY1 = (double)Y1 - (350.0 - (double)Y2)/(215.0) * 3.0;
        realZ = (((315 - adjustedY1)/28) * 35.0);
    }
    
    
    // 1, 1 to 2, 2
    if(X1 > 520 && X1 <= 545 && Y1 > 258 && Y1 <= 287 && Y2 <= 296 && Y2 > 242)
    {
        realX = ((296.0 - (double)Y2)/54.0 * 35.0 + 35.0) - 70.0;
        double adjustedY1 = (double)Y1 - (350.0 - (double)Y2)/(215.0) * 3.0;
        realZ = (((315 - adjustedY1)/28) * 35.0);
    }
    
    
    // 2, 1 to 3, 2
    if(X1 > 545 && X1 <= 573 && Y1 > 250 && Y1 <= 290 && Y2 <= 242 && Y2 > 190)
    {
        realX = ((242.0 - (double)Y2)/52.0 * 35.0 + 70.0) - 70.0;
        double adjustedY1 = (double)Y1 - (350.0 - (double)Y2)/(215.0) * 3.0;
        realZ = (((315 - adjustedY1)/28) * 35.0);
    }
    
    
    // 3, 1 to 4, 2
    if(X1 > 573 && X1 <= 606 && Y1 > 252 && Y1 <= 290 && Y2 <= 190 && Y2 > 135)
    {
        realX = ((190.0 - (double)Y2)/55.0 * 35.0 + 105.0) - 70.0;
        double adjustedY1 = (double)Y1 - (350.0 - (double)Y2)/(215.0) * 3.0;
        realZ = (((315 - adjustedY1)/28) * 35.0);
    }
    
    
}