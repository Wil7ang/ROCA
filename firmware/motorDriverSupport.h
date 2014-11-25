#include "SPISupport.h"

#define FOREARM_DIR _BV(PD2)
#define FOREARM_EN _BV(PD1)

#define SHOULDER_DIR _BV(PD5)
#define SHOULDER_EN _BV(PD4)

void setMotorVelocity(int dutyForearm, int dutyShoulder);

extern int enc_count;

#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX(a,b) (((a)>(b))?(a):(b))

//Setup motor drivers for forearm and shoulder
void setupMotorDriver()
{
    SPCR |= _BV(SPE) | _BV(MSTR) | _BV(SPR0) | _BV(SPR1) | _BV(CPHA);
    _delay_ms(250);


    unsigned char sendData[4] = {0x30,0x00, 0x00, 0x00};
    SPITransmitMultiple(MOTOR_DRIVER, sendData, 4);
    _delay_ms(500);

    SPCR |= _BV(SPE) | _BV(MSTR) | _BV(SPR0) | _BV(SPR1) | _BV(CPOL) | _BV(CPHA);

    PORTD |= _BV(PD4);
    PORTD |= _BV(PD1);
    OCR3A = 0;
    OCR3B = 0;

    _delay_ms(500);

    zeroForearm();
}

void zeroForearm()
{
    setMotorVelocity(-16000, 0);
    _delay_ms(800);
    enc_count = 0;
    _delay_ms(500);

    setMotorVelocity(0,0);
}

//Duty cycle of the motor from 0.0 to 1.0
void setMotorVelocity(int dutyForearm, int dutyShoulder)
{
    OCR3A = MIN(MAX(abs(dutyForearm), 0), 16000);//(abs(dutyForearm)>16000)?16000:abs(dutyForearm);
    if(dutyForearm < 0)
    {
        PORTD |= FOREARM_DIR;
        PORTD |= FOREARM_EN;
    }
    else if(dutyForearm > 0)
    {
        PORTD &= ~FOREARM_DIR;
        PORTD |= FOREARM_EN;
    }
    else
    {
        PORTD &= ~(FOREARM_DIR | FOREARM_EN);
    }


    /*if(abs(dutyShoulder) < 8000 && abs(dutyShoulder) > 3000)
        dutyShoulder = dutyShoulder/abs(dutyShoulder) * 8000;
*/
    OCR3B = MIN(MAX(abs(dutyShoulder), 0), 16000);
    if(dutyShoulder < 0)
    {
        PORTD &= ~SHOULDER_DIR;
        PORTD |= SHOULDER_EN;
    }
    else if(dutyShoulder > 0)
    {
        PORTD |= SHOULDER_DIR;
        PORTD |= SHOULDER_EN;
    }
    else
    {
        PORTD &= ~(SHOULDER_DIR | SHOULDER_EN);
    }
}