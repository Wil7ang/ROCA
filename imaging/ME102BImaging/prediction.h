#include <cmath>
#include <complex>

using namespace std;

#define FOREARM_LENGTH 0.290
#define UPPERARM_LENGTH 0.215
#define ARM_HOME_POSITION_RADIUS 0.361 //sqrt(FOREARM_LENGTH^2 + UPPERARM_LENGTH^2)
#define REACHABLE_RADIUS FOREARM_LENGTH+UPPERARM_LENGTH
#define GRAVITY 9.8

struct Point3DD
{
    Point3DD(){ x = y = z = 0.0;}
    Point3DD(double x_, double y_, double z_){x = x_;y=y_;z=z_;}
    ~Point3DD(){}
    double x;
    double y;
    double z;
};

struct ArmAngles
{
    ArmAngles(){valid=base=shoulder=elbow=wristA=wristB=0;}
    ArmAngles(int base_, int shoulder_, int elbow_, int wristA_, int wristB_)
    {
        base = base_;
        shoulder = shoulder_;
        elbow = elbow_;
        wristA = wristA_;
        wristB = wristB_;
        valid = true;
    }
    ~ArmAngles(){}
    int base;
    int shoulder;
    int elbow;
    int wristA;
    int wristB;
    bool valid;
};

vector<complex<double> > CubicSolver(complex<double> a, complex<double> b, complex<double> c, complex<double> d)
{
    complex<double> f = ((3.0*c/a) - (pow(b,2.0)/pow(a,2.0)))/3.0;
    complex<double> g = ((2.0*pow(b,3.0)/pow(a,3.0))-(9.0*b*c/pow(a,2.0))+(27.0*d/a))/27.0;
    complex<double> h = (pow(g,2.0)/4.0)+(pow(f,3.0)/27.0);

    complex<double> imaginary(0,1);

    if(h.real() > 0.0 || h.imag() > 0.0)
    {
        complex<double> R = -(g/2.0) + sqrt(h);
        complex<double> S = pow(R,1.0/3.0);
        complex<double> T = -(g/2.0) - sqrt(h);
        complex<double> U = pow(T,1.0/3.0);

        vector<complex<double> > solutions;
        solutions.push_back((S+U) - (b/(3.0*a)));
        solutions.push_back(-(S+U)/2.0 - (b/(3.0*a)) + imaginary*(S-U)*sqrt(3.0)/2.0);
        solutions.push_back(-(S+U)/2.0 - (b/(3.0*a)) - imaginary*(S-U)*sqrt(3.0)/2.0);

        return solutions;
    }
    else if(h.real() <=0 && h.imag() == 0 &&
            f.real() != 0 && f.imag() != 0 &&
            g.real() != 0 && g.imag() != 0)
    {
        complex<double> i = sqrt((pow(g,2.0)/4.0)-h);
        complex<double> j = pow(i,1.0/3.0);
        complex<double> k = acos(-(g/(2.0*i)));
        complex<double> L = -j;
        complex<double> M = cos(k/3.0);
        complex<double> N = sqrt(3.0)*sin(k/3.0);
        complex<double> P = -(b/(3.0*a));

        vector<complex<double> > solutions;
        solutions.push_back(2.0*imaginary*M - (b/(3.0*a)));
        solutions.push_back(-imaginary*(M+N)+P);
        solutions.push_back(L*(M-N)+P);

        return solutions;
    }
    else
    {
        vector<complex<double> > solutions;
        solutions.push_back(-pow(d/a,1.0/3.0));
        solutions.push_back(-pow(d/a,1.0/3.0));
        solutions.push_back(-pow(d/a,1.0/3.0));

        return solutions;
    }
}

vector<complex<double> > QuarticSolver(double a, double b, double c, double d, double e)
{
    int i = 0;

    e = e/a;
    d = d/a;
    c = c/a;
    b = b/a;
    a = 1.0;

    complex<double> f = c - (3.0*pow(b,2)/8.0);
    complex<double> g = d + (pow(b,3.0)/8.0) - (b*c/2.0);
    complex<double> h = e - (3.0*pow(b,4.0)/256.0) + (pow(b,2.0)*c/16.0) - (b*d/4.0);

    complex<double> A = f/2.0;
    complex<double> B = (pow(f,2.0)-4.0*h)/16.0;
    complex<double> C = -pow(g,2.0)/64.0;

    vector<complex<double> > Y = CubicSolver(1.0, A, B, C);

    complex<double> p = 0.0;
    complex<double> q = 0.0;

    double compareValue = 0.00000001;

    for(i = 0; i<Y.size();i++)
    {
        if(p.real() == 0 && p.imag() == 0 && abs(-Y[i]) > 0.0)
        {
            p = sqrt(Y[i]);
            continue;
        }

        if(q.real() == 0 && q.imag() == 0 && abs(-Y[i]) > 0.0)
        {
            q = sqrt(Y[i]);
            break;
        }
    }

    complex<double> r = -g/(8.0*p*q);
    complex<double> s = b/(4.0*a);

    vector<complex<double> > solutions;
    solutions.push_back(p+q+r-s);
    solutions.push_back(p-q-r-s);
    solutions.push_back(-p+q-r-s);
    solutions.push_back(-p-q+r-s);

    return solutions;
}

Point3DD FindImpactPoint(double v_xi, double v_yi, double v_zi, double x_0, double y_0, double z_0, double R)
{
    int i = 0;

    double a = (1.0/4.0) * pow(GRAVITY, 2);
    double b = -GRAVITY*v_zi;
    double c = pow(v_xi,2) + pow(v_yi,2) + pow(v_zi,2) - GRAVITY*z_0;
    double d = 2*v_xi*x_0 + 2*v_yi*y_0 + 2*v_zi*z_0;
    double e = pow(x_0,2) + pow(y_0,2) + pow(z_0,2) - pow((R-0.001),2);

    vector<complex<double> > solutions = QuarticSolver(a,b,c,d,e);

    double maxTime = 99999;
    double compareValue = 0.000001;

    for(i = 0; i<solutions.size();i++)
    {
        if(abs(solutions[i].imag()) <= compareValue && solutions[i].real() < maxTime)
            maxTime = solutions[i].real();
    }

    if(maxTime < 10.0)
        return Point3DD( (v_xi*maxTime + x_0),
                         (v_yi*maxTime + y_0),
                         (-0.5*GRAVITY*pow(maxTime,2.0) + v_zi*maxTime + z_0));
    else
        return Point3DD(-1,-1,-1);
}

Point3DD FindCatchLocation(double v_xi, double v_yi, double v_zi, double x_0, double y_0, double z_0)
{
    return FindImpactPoint(v_xi,v_yi,v_zi,x_0,y_0,z_0,REACHABLE_RADIUS-0.2);
}

//Analytical IK Solver
ArmAngles GetArmAngles(Point3DD position)
{
    double d = sqrt(pow(position.x,2) + pow(position.y,2) + pow(position.z,2));

    if(d <= REACHABLE_RADIUS && d > UPPERARM_LENGTH)
    {
        double a = acos((pow(d,2) + pow(UPPERARM_LENGTH,2) - pow(FOREARM_LENGTH,2))/(2*d*UPPERARM_LENGTH));
        double A = atan(position.z/sqrt(pow(position.x,2)+pow(position.y,2)));

        double b = acos((pow(FOREARM_LENGTH,2) + pow(UPPERARM_LENGTH,2) - pow(d,2))/(2*UPPERARM_LENGTH*FOREARM_LENGTH));
        double alph = 2*M_PI - M_PI/2 - b;
        double gamma = 0;

        double beta = A + a;
        alph = M_PI - alph;

        if(position.y > 0)
        {
            gamma = atan(position.x/position.y);
        }
        else if(position.y >= 0 && position.x > 0)
        {
            gamma = M_PI/2 - atan(position.y/position.x);
        }
        else
        {
            gamma = M_PI;
        }

        return ArmAngles(gamma * 1800 / M_PI,
                         beta * 1800 / M_PI,
                         alph * 1800 / M_PI,
                         900,900);
    }
    else
    {
        return ArmAngles();
    }
}
