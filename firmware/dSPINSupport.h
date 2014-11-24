#include "SPISupport.h"

void hardStop();

//Setup stepper motor driver
int setupStepperMotorDriver()
{
    printf("dSPIN Initializing!\n");
    SPITransmitReceive(STEPPER_DRIVER, 0x38);
    unsigned char one = SPITransmitReceive(STEPPER_DRIVER, 0x0);
    unsigned char two = SPITransmitReceive(STEPPER_DRIVER, 0x0);

    if(one == 0x2E && two == 0x88)
    {
        printf("dSPIN Initialized! %x %x\r\n", one, two);
    }
    else
    {
        printf("Failed!!! %x %x \r\n", one, two);
    }

    // //Set step mode
    SPITransmitReceive(STEPPER_DRIVER, 0x16);
    SPITransmitReceive(STEPPER_DRIVER, 0x0);

    //Set max speed
    SPITransmitReceive(STEPPER_DRIVER, 0x07);
    SPITransmitReceive(STEPPER_DRIVER, 0x0);
    SPITransmitReceive(STEPPER_DRIVER, 0x1F);

    SPITransmitReceive(STEPPER_DRIVER, 0x13);
    SPITransmitReceive(STEPPER_DRIVER, 0x0F);

    SPITransmitReceive(STEPPER_DRIVER, 0x18);
    SPITransmitReceive(STEPPER_DRIVER, 0x2E);
    SPITransmitReceive(STEPPER_DRIVER, 0x08);

    SPITransmitReceive(STEPPER_DRIVER, 0x09);
    SPITransmitReceive(STEPPER_DRIVER, 0x0A);

    SPITransmitReceive(STEPPER_DRIVER, 0x0A);
    SPITransmitReceive(STEPPER_DRIVER, 0x0A);

    SPITransmitReceive(STEPPER_DRIVER, 0x38);
    unsigned char t = SPITransmitReceive(STEPPER_DRIVER, 0x0);
    unsigned char r = SPITransmitReceive(STEPPER_DRIVER, 0x0);
    printf("%x %x\r\n", t, r);


    SPITransmitReceive(STEPPER_DRIVER, 0xD8);
    SPITransmitReceive(STEPPER_DRIVER, 0x0);

    return 0;
}

int32_t AngleToSteps(int32_t angle)
{
    return (angle * 3 * 5)/90;
}

int32_t getStepperPosition()
{
    int32_t pos = 0;
    SPITransmitReceive(STEPPER_DRIVER, 0x21);
    pos |= (SPITransmitReceive(STEPPER_DRIVER, 0x0)<<16);
    pos |= (SPITransmitReceive(STEPPER_DRIVER, 0x0)<<8);
    pos |= SPITransmitReceive(STEPPER_DRIVER, 0x0);

    if((pos>>21) == 1)
    {
        pos |= 0xFFC00000;
    }

    return pos;
}

void hardStop()
{
    SPITransmitReceive(STEPPER_DRIVER, 0xB8);
    SPITransmitReceive(STEPPER_DRIVER, 0x0);
}

//Stepper motor control hopefully this value is between -900 and 900
void setStepperAngle(int angle)
{
    int32_t theAngle = AngleToSteps(angle);

    
    if(theAngle > getStepperPosition())
    {
        SPITransmitReceive(STEPPER_DRIVER, 0x69);
        SPITransmitReceive(STEPPER_DRIVER, (theAngle >> 16) & 0x3F);
        SPITransmitReceive(STEPPER_DRIVER, (theAngle >> 8 ) & 0xFF);
        SPITransmitReceive(STEPPER_DRIVER, (theAngle & 0xFF));
    }
    else
    {
        SPITransmitReceive(STEPPER_DRIVER, 0x68);
        SPITransmitReceive(STEPPER_DRIVER, (theAngle >> 16) & 0x3F);
        SPITransmitReceive(STEPPER_DRIVER, (theAngle >> 8 ) & 0xFF);
        SPITransmitReceive(STEPPER_DRIVER, (theAngle & 0xFF));
    }
}
