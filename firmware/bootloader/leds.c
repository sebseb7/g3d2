#include <inttypes.h>
#include <avr/io.h>
#include <util/delay.h>
#include <stdlib.h>

#include "leds.h"


void leds_init(void)
{
	PORTC = 0x00;
	PORTB = 0x00;

	PORTD &= ~((1<<2)|(1<<3)|(1<<6)|(1<<7));

	DDRC = 0xFF;
	DDRB = 0xFF;

	DDRD |= ((1<<2)|(1<<3)|(1<<6)|(1<<7));


	TCCR1A |= (1<<COM1A1)|(1<<WGM11);
	TCCR1B |= (1<<WGM12)|(1<<WGM13)|(1<<CS10);
	OCR1A  = 0xff;
	ICR1  = 0xfff;


	while(1)
	{
		PORTC |= (1<<4);

		_delay_ms(500);

		PORTC &= ~(1<<4);

		_delay_ms(500);
	}
}
