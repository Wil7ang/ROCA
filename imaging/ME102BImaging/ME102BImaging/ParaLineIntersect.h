#include <cmath>
using namespace std;

void LineLine(double D1,double E1, double D2, double E2, int &x, int &y)
{
x=(E2-E1)/(D1-D2);
y=D1*x+E1;}