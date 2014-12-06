#ifndef SPISUPPORT_H
#define SPISUPPORT_H

//SPI device selects
#define MOTOR_DRIVER_CS _BV(PD6)
#define SHOULDER_ENCODER_CS _BV(PD3)
#define STEPPER_DRIVER_CS _BV(PB0)

//SPI defines
#define MOSI _BV(PB2)
#define MISO _BV(PB3)
#define SCK _BV(PB1)
#define SS _BV(PB0)

typedef enum{MOTOR_DRIVER,SHOULDER_ENCODER,STEPPER_DRIVER} SPIselect;

void initializeSPI()
{
    //Setup pin directions for Port D (CS)
    DDRD |= SHOULDER_ENCODER_CS | MOTOR_DRIVER_CS;
    PORTD = 0;

    //Setup SPI
    DDRB |= MOSI | SCK | SS;
    SPCR |= _BV(SPE) | _BV(MSTR) | _BV(SPR0) | _BV(SPR1) | _BV(CPHA);

    PORTB |= SS;
    PORTD |= SHOULDER_ENCODER_CS | MOTOR_DRIVER_CS;
}

void ToggleCS(SPIselect cs)
{
    switch(cs)
    {
        case MOTOR_DRIVER: PORTD ^= MOTOR_DRIVER_CS; break;
        case SHOULDER_ENCODER: PORTD ^= SHOULDER_ENCODER_CS; break;
        case STEPPER_DRIVER: PORTB ^= STEPPER_DRIVER_CS; break;
        default: break;
    }
}

void SetCS(SPIselect cs, int val)
{
    if(val)
    {
        switch(cs)
        {
            case MOTOR_DRIVER: PORTD |= MOTOR_DRIVER_CS; break;
            case SHOULDER_ENCODER: PORTD |= SHOULDER_ENCODER_CS; break;
            case STEPPER_DRIVER: PORTB |= STEPPER_DRIVER_CS; break;
            default: break;
        }
    }
    else
    {
        switch(cs)
        {
            case MOTOR_DRIVER: PORTD &= ~MOTOR_DRIVER_CS; break;
            case SHOULDER_ENCODER: PORTD &= ~SHOULDER_ENCODER_CS; break;
            case STEPPER_DRIVER: PORTB &= ~STEPPER_DRIVER_CS; break;
            default: break;
        }
    }
}

unsigned char SPITransmitReceive(SPIselect device, unsigned char data)
{
    SetCS(device, 0);
    SPDR = data;
    while(!(SPSR & _BV(SPIF)));
    SetCS(device, 1);
    return SPDR;
}

unsigned char* SPITransmitMultiple(SPIselect device, unsigned char *data, unsigned int length)
{
    unsigned int i = 0;
    unsigned char *returnData = 0;
    returnData = malloc(sizeof(unsigned char) * length);
    SetCS(device, 0);
    for(i = 0; i < length; i++)
    {
        SPDR = data[i];
        while(!(SPSR & _BV(SPIF)));
        returnData[i] = SPDR;
    }
    SetCS(device, 1);

    return returnData;
}

unsigned char SPITransmitReceiveNext(SPIselect device, unsigned char data)
{
    SetCS(device, 0);
    SPDR = data;
    while(!(SPSR & _BV(SPIF)));
    SPDR = 0x0;
    while(!(SPSR & _BV(SPIF)));    
    SetCS(device, 1);
    return SPDR;
}

#endif