/* Name: main.c
 * Author: Wilson Tang
 * Copyright: 2014
 */

#include <avr/io.h>
#include <util/delay.h>
#include <avr/wdt.h>
void dostuff()
{
    DDRB |= _BV(PB2) | _BV(PB1) | _BV(PB0);
    SPCR = _BV(SPE) | _BV(MSTR) | _BV(SPR0) | _BV(CPOL);

    PORTB |= _BV(PB0);
}

int main(void)
{
    dostuff();
    //PORTB |= _BV(PB0);

    while(1)
    {
        PORTB &= ~_BV(PB0);
        SPDR = 0x83;
        while(!(SPSR & _BV(SPIF)));
        PORTB |= _BV(PB0);
        _delay_ms(100);
    }

    return 0;
}
