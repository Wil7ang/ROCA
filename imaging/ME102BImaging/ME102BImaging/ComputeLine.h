using namespace std;

void ComputeLine(int x1, int y1, int x2, int y2, double &D, double &E)
{
D=double((y2 - y1))/double((x2 - x1));
E=double(y1)-D*double(x1);
}