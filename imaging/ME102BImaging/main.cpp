#include <SerialStream.h>
#include <stdio.h>
#include <unistd.h>

#include <iostream>

#include "prediction.h"



using namespace LibSerial;

void moveROCA(SerialStream &connection, int16_t base, int16_t shoulder, int16_t elbow, int16_t wristA, int16_t wristB)
{
    int i = 0;
    unsigned char command[15] = {0x81, (base>>8) & 0xFF, base & 0xFF,
                                 0x82, (shoulder>>8) & 0xFF, shoulder & 0xFF,
                                 0x83, (elbow>>8) & 0xFF, elbow & 0xFF,
                                 0x84, (wristA>>8) & 0xFF, wristA & 0xFF,
                                 0x85, (wristB>>8) & 0xFF, wristB & 0xFF,};

    for(i = 0;i<16;i+=3)
    {
        connection << command[i];
        usleep(10000);
        connection << command[i+1];
        usleep(10000);
        connection << command[i+2];
        usleep(10000);
    }
}


int main()
{
    Point3DD location = FindCatchLocation(-7.0,0.0,6.45,9.0,0.0,0.0);
    printf("%f %f %f", location.x, location.y, location.z);

//    SerialStream rocaConnection( "/dev/ttyUSB0",
//                            SerialStreamBuf::BAUD_19200,
//                            SerialStreamBuf::CHAR_SIZE_8,
//                            SerialStreamBuf::PARITY_NONE,
//                            1,
//                           SerialStreamBuf::FLOW_CONTROL_NONE );
//
//    if(!rocaConnection.IsOpen())
//    {
//        printf("Failed to open serial port!\n");
//        return -1;
//    }
//
//    rocaConnection.Close();
}
