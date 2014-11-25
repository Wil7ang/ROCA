/* Name: main.c
 * Author: Wilson Tang
 * Copyright: 2014
 */

#include <avr/io.h>
#include <util/delay.h>
#include <inttypes.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>

#include <math.h>


#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>

#include "SPISupport.h"
#include "dSPINSupport.h"
#include "motorDriverSupport.h"
#include "UARTSupport.h"

#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX(a,b) (((a)>(b))?(a):(b))
 

//PID Control definitions
#define SHOULDER_P 20
#define SHOULDER_I 10//16
#define SHOULDER_D 5

#define FOREARM_P 30
#define FOREARM_I 5
#define FOREARM_D 0

#define UPPER_ARM_LENGTH 0.215 //0.2m
#define FOREARM_LENGTH 0.128
#define REACH_RADIUS UPPER_ARM_LENGTH+FOREARM_LENGTH


#define SERVO_SIGNAL_A _BV(PB5)
#define SERVO_SIGNAL_B _BV(PB6)

#define FOREARM_MOTOR_PWM _BV(PE3)
#define SHOULDER_MOTOR_PWM _BV(PE4)

#define FOREARM_DUTY_H OCR3AH
#define FOREARM_DUTY_L OCR3AL

#define SHOULDER_DUTY_H OCR3BH
#define SHOULDER_DUTY_L OCR3BL


#define SERVO_POSITION_A_H OCR1AH
#define SERVO_POSITION_A_L OCR1AL

#define SERVO_POSITION_B_H OCR1BH
#define SERVO_POSITION_B_L OCR1BL

#define UINT16T_MAX 65536.0f

#define SHOULDER_ENDPOINT_F 962L
#define SHOULDER_ENDPOINT_B 30L

//UART defines
#define FOSC 16000000// Clock Speed
#define BAUD 19200
#define UARTCLOCK FOSC/16/BAUD-1

void setServoPosition(uint16_t servoA, uint16_t servoB);

typedef struct
{
    int16_t forearmDuty;
    int16_t shoulderDuty;
    int16_t baseAngle;
} motorParameters;

int enc_count = 0;

void encoder_isr() {
    static int8_t lookup_table[] = {0,-1,1,0,1,0,0,-1,-1,0,0,1,0,1,-1,0};
    static uint8_t enc_val = 0;
    
    enc_val = enc_val << 2;
    enc_val = enc_val | ((PINE & 0b11000000) >> 6);
 
    enc_count = enc_count + lookup_table[enc_val & 0b1111];
}

ISR(INT7_vect)
{
    encoder_isr();
}

ISR(INT6_vect)
{
    encoder_isr();
}



void initPins()
{
    //Setup servo PWM outputs
    DDRB |= SERVO_SIGNAL_B | SERVO_SIGNAL_A; //Enable outputs for servo signal lines
    TCCR1A |= _BV(COM1A1) | _BV(COM1B1) | _BV(WGM11); //Non-nverting mode fast pwm
    TCCR1A &= ~_BV(COM1A0);
    TCCR1B |= _BV(WGM13) | _BV(WGM12) | _BV(CS11); //Clock pre-scaler to 8
    //Set 20ms period for servo PWM
    ICR1 = 40000;

    setServoPosition(900, 900);

    //Setup forearm and shoulder motor PWM outputs
    DDRE |= FOREARM_MOTOR_PWM | SHOULDER_MOTOR_PWM;
    TCCR3A |= _BV(COM3A1) | _BV(COM3B1) | _BV(WGM31); //Non-inverting mode fast pwm
    TCCR3B |= _BV(WGM33) | _BV(WGM32) | _BV(CS30); //No prescale on clock

    //Set 1KHz PWM frequency
    ICR3H = 0x3E;
    ICR3L = 0x80;

    setMotorVelocity(0, 0);

    initializeSPI();

    DDRD |= FOREARM_EN | FOREARM_DIR | SHOULDER_EN | SHOULDER_DIR;

    //Setup stepper motor inputs
    DDRC |= _BV(PC6); //Enable output to STCK
    PORTC = 0;

    EICRB |= _BV(ISC70) | _BV(ISC60);
    EIMSK |= _BV(INT7) | _BV(INT6);
    sei();
}



uint16_t GetShoulderEncoderPosition()
{
    unsigned int i = 0;
    uint16_t data = 0;
    SetCS(SHOULDER_ENCODER, 0);
    SPDR = 0x0;
    while(!(SPSR & _BV(SPIF)));

    data = data | (SPDR<<8);

    SPDR = 0x0;
    while(!(SPSR & _BV(SPIF)));   
    data = data | SPDR; 
    SetCS(SHOULDER_ENCODER, 1);

    data = data| SPDR;

    data = (data<<1)>>6;

    return data;    
}

uint16_t currentServoPositionA = 900;
uint16_t currentServoPositionB = 900;

//Set degrees in 0.1 degree increments
void setServoPosition(uint16_t servoA, uint16_t servoB)
{
    currentServoPositionA = servoA;
    currentServoPositionB = servoB;

    uint16_t pos = 1200U + (servoA * 18)/9;
    OCR1A = pos;

    pos = 1800U + (servoB * 4)/3;
    OCR1B = pos;
}

motorParameters UpdateControls(int baseAngle, int currentShoulderAngle, int shoulderAngleSet, int shoulderIntegral, int shoulderDerivative, int currentForearmAngle, int forearmAngleSet, int forearmIntegral, int forearmDerivative)
{
    motorParameters duties;

    uint16_t forearmDutyMagnitude = FOREARM_P * (uint16_t)abs(forearmAngleSet - currentForearmAngle);
    if(forearmDutyMagnitude > 16000)
        forearmDutyMagnitude = 16000;

    int16_t forearmDutySign = (forearmAngleSet - currentForearmAngle)/abs(forearmAngleSet - currentForearmAngle);
    //PID for arm motion
    duties.forearmDuty =  forearmDutySign * forearmDutyMagnitude;
    duties.forearmDuty += FOREARM_I * forearmIntegral;
    duties.forearmDuty += FOREARM_D * forearmDerivative;


    uint16_t shoulderDutyMagnitude = SHOULDER_P * (uint16_t)abs(shoulderAngleSet - currentShoulderAngle);
    if(shoulderDutyMagnitude > 16000)
        shoulderDutyMagnitude = 16000;

    int16_t shoulderDutySign = (shoulderAngleSet - currentShoulderAngle)/abs(shoulderAngleSet - currentShoulderAngle);

    duties.shoulderDuty = shoulderDutySign * shoulderDutyMagnitude;

    duties.shoulderDuty += SHOULDER_I * shoulderIntegral;

    duties.shoulderDuty += SHOULDER_D * shoulderDerivative;
    printf("%i %i\r\n", duties.shoulderDuty, duties.shoulderDuty + 16 * shoulderIntegral);

    return duties;
}

//Arm goes from -10.2 degrees to 190.2 degrees
long int convertShoulderPositionToAngle(long int position)
{
    long int angle = ((1902L - (-102L)) * (position - SHOULDER_ENDPOINT_B) / (SHOULDER_ENDPOINT_F - SHOULDER_ENDPOINT_B)) + (-102L); //Check for overflow
    return angle;/*2004 - (angle + 102) - 102;*/
}

//Forearm goes from -40 to 220
long int convertForearmPositionToAngle(long int position)
{
    return ((2200L - (-400L)) * (position - 0L) / (4491L)) + (-400L);
}

void printByte(char byte)
{
    int i;
  for (i = 0; i < 8; i++) {
      printf("%d", !!((byte << i) & 0x80));
  }
}

int main()
{
    int j = 0;
    initPins();
    setupMotorDriver();
    

    usart_init ( UARTCLOCK );

    int shoulderSetAngle = 1350;
    int forearmSetAngle = -400;

    int errorHistoryShoulder[10] = {0,0,0,0,0,0,0,0,0,0};
    int errorHistoryShoulderIndex = 0;

    int errorHistoryForearm[10] = {0,0,0,0,0,0,0,0,0,0};
    int errorHistoryForearmIndex = 0;

    unsigned int stringPosition = 0;


    _delay_ms(500);

    setupStepperMotorDriver();

    _delay_ms(500);

    setServoPosition(900, 1800);
    _delay_ms(100);

    hardStop();


    for(;;)
    {
        unsigned char received[3];
        if(usart_kbhit())
        {
            if(stringPosition < 3)
            {
                received[stringPosition] = usart_getchar();
                stringPosition++;
            }
        }


        int ang = GetShoulderEncoderPosition();
        int shoulderCurrentAngle = convertShoulderPositionToAngle(ang);

        int forearmCurrentAngle = convertForearmPositionToAngle(enc_count);

        errorHistoryShoulder[errorHistoryShoulderIndex] = (2004 - (shoulderSetAngle + 102) - 102) - shoulderCurrentAngle;
        int errorChange = errorHistoryShoulder[errorHistoryShoulderIndex] - errorHistoryShoulder[(errorHistoryShoulderIndex+9)%10];


        errorHistoryForearm[errorHistoryForearmIndex] = forearmSetAngle - forearmCurrentAngle;
        int errorChangeForearm = errorHistoryForearm[errorHistoryForearmIndex] - errorHistoryForearm[(errorHistoryForearmIndex+9)%10];

        errorHistoryShoulderIndex++;
        if(errorHistoryShoulderIndex > 9)
            errorHistoryShoulderIndex = 0;

        errorHistoryForearmIndex++;
        if(errorHistoryForearmIndex > 9)
            errorHistoryForearmIndex = 0;

        int errorAverage = (errorHistoryShoulder[0] +
                            errorHistoryShoulder[1] +
                            errorHistoryShoulder[2] +
                            errorHistoryShoulder[3] +
                            errorHistoryShoulder[4] +
                            errorHistoryShoulder[5] +
                            errorHistoryShoulder[6] +
                            errorHistoryShoulder[7] +
                            errorHistoryShoulder[8] +
                            errorHistoryShoulder[9])/10;

        int errorAverageForearm = (errorHistoryForearm[0] +
                                   errorHistoryForearm[1] +
                                   errorHistoryForearm[2] +
                                   errorHistoryForearm[3] +
                                   errorHistoryForearm[4] +
                                   errorHistoryForearm[5] +
                                   errorHistoryForearm[6] +
                                   errorHistoryForearm[7] +
                                   errorHistoryForearm[8] +
                                   errorHistoryForearm[9])/10;

        //printf("%i\r\n", errorAverage);

        motorParameters motorState = UpdateControls(0, 
            shoulderCurrentAngle, 
            2004 - (shoulderSetAngle + 102) - 102, 
            errorAverage, 
            errorChange, 
            forearmCurrentAngle, 
            forearmSetAngle,
            errorAverageForearm,
            errorChangeForearm);

        setMotorVelocity(motorState.forearmDuty, motorState.shoulderDuty);


        //Handle serial commands. first byte is the command byte, next two byte is the value
        if(stringPosition == 3)
        {
            stringPosition = 0;
            //printf("%x %x %x\r\n", received[0], received[1], received[2]);  

            int16_t argument = 0;
            argument |= (received[1] << 8) | (received[2]);

            switch(received[0])
            {
                case 0b10000001:
                    //printf("Base! %i %x %x\r\n", argument, received[1], received[2]);
                    printf("Base: %i\r\n", argument);
                    setStepperAngle(argument);
                    break;
                case 0b10000010:
                    printf("Shoulder! %i\r\n", argument);
                    shoulderSetAngle = argument;
                    break;
                case 0b10000011:
                    printf("Elbow! %i\r\n", argument);
                    forearmSetAngle = argument;
                    break;
                case 0b10000100:
                    printf("Wrist A! %i\r\n", argument);
                    setServoPosition(argument, currentServoPositionB);
                    break;
                case 0b10000101:
                    printf("Wrist B! %i\r\n", argument);
                    setServoPosition(currentServoPositionA, argument);
                default: break;
            }
        }

        //shoulderSetAngle = MIN(MAX(shoulderSetAngle, -102), 1902);
        //forearmSetAngle = MIN(MAX(forearmSetAngle, -400), 2200);

        //printf("%i %i %i\n", forearmCurrentAngle, forearmSetAngle, motorState.forearmDuty);
        //printf("%i %i %i %i\r\n", ang, shoulderCurrentAngle, 2004 - (shoulderSetAngle + 102) - 102, motorState.shoulderDuty);
        //printf("Ang: %i\r\n", ang);
        // for(j = 0; j < 10; j++)
        // {
        //     printf("%i ", errorHistoryShoulder[j]);
        // }
        // printf("\n\n");
    }

    return 0;
}
