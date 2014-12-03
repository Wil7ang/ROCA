#include <string>
#include <stdio.h>
using namespace std;

void GetRealWorldCoordinates(double &realX, double &realY, double &realZ, int X1, int Y1, int X2, int Y2)
{
    // 0, 0 to 1, 1
    if(X1 > 553 && X1 <= 584 && Y1 > 306 && Y1 <= 357 && Y2 <= 269 && Y2 > 222)
    {
        realX = ((269.0 - (double)Y2)/47.0 * 30.0 + 0.0) - 30.0;
        double adjustedY1 = (double)Y1 - (269.0 - (double)Y2)/(90) * 16;
        realZ = 30 - ((adjustedY1 - 306)/(42) * 30);
    }
    
    
    // 1, 0 to 2, 1
    if(X1 > 584 && X1 <= 624 && Y1 > 310 && Y1 <= 364 && Y2 <= 222 && Y2 > 179)
    {
        realX = ((222.0 - (double)Y2)/43.0 * 30.0 + 30.0) - 30.0;
        double adjustedY1 = (double)Y1 - (269.0 - (double)Y2)/(90) * 16;
        realZ = 30 - ((adjustedY1 - 306)/(42) * 30);
    }
    
    
}
