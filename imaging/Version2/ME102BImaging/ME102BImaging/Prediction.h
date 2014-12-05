#ifndef PREDICTION_H
#define PREDICTION_H

#include <cmath>
#include <complex>
#include <vector>

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
        else if(position.y <= 0 && position.x > 0)
        {
            gamma = M_PI/2 - atan(position.y/position.x);
        }
        else
        {
            gamma = M_PI;
        }

        return ArmAngles((int)(gamma * 1800 / M_PI),
                         (int)(beta * 1800 / M_PI),
                         (int)(alph * 1800 / M_PI),
                         900,900);
    }
    else
    {
        return ArmAngles();
    }
}


float determinant3x3(float a, float b, float c,
                     float d, float e, float f,
                     float g, float h, float i)
{
    return a*e*i + b*f*g + c*d*h - a*f*h - b*d*i - c*e*g;
}

void GetParabola(double &A, double &B, double &C, vector<pair<int,int> > pointStorage, int currentPointSize)
{
    float detA = determinant3x3(pointStorage[2].second, pointStorage[2].first, 1,
                                pointStorage[5].second, pointStorage[5].first, 1,
                                pointStorage[currentPointSize].second, pointStorage[currentPointSize].first, 1);
    
    float detB = determinant3x3(pow((float)pointStorage[2].first,2), pointStorage[2].second, 1,
                                pow((float)pointStorage[5].first,2), pointStorage[5].second, 1,
                                pow((float)pointStorage[currentPointSize].first,2), pointStorage[currentPointSize].second, 1);
    
    float detC = determinant3x3(pow((float)pointStorage[2].first,2), pointStorage[2].first, pointStorage[2].second,
                                pow((float)pointStorage[5].first,2), pointStorage[5].first, pointStorage[5].second,
                                pow((float)pointStorage[currentPointSize].first,2), pointStorage[currentPointSize].first, pointStorage[currentPointSize].second);
    
    float detD = determinant3x3(pow((float)pointStorage[2].first,2), pointStorage[2].first, 1,
                                pow((float)pointStorage[5].first,2), pointStorage[5].first, 1,
                                pow((float)pointStorage[currentPointSize].first,2), pointStorage[currentPointSize].first, 1);
    
    A = detA/detD;
    B = detB/detD;
    C = detC/detD;
}

#endif