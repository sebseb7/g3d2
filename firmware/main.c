#include <inttypes.h>
#include <avr/io.h>
#include <avr/interrupt.h>
//#include <util/delay.h>
#include <stdlib.h>

#include "main.h"
#include "pins.h"
#include "plasma_seb.h"
//#include "helix.h"


uint8_t active_col = 1;

uint8_t leds[64] = {
	0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,
};

uint8_t volatile leds_buf[64] = {
	0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,
};

uint8_t cycle = 0;
uint8_t volatile led_mode = 0;

uint8_t volatile frame_update = 0;
ISR (TIMER0_OVF_vect)
{


	cycle++;

	if(
		(cycle != 0)&&
		(cycle != 1)&&
		(cycle != 2)&&
		(cycle != 4)&&
		(cycle != 8)&&
		(cycle != 20)&&
		(cycle != 40)&&
		(cycle != 70)
	)
	{
		return;
	}
//		ROW0_OFF;

	

	if(cycle == 70)
	{
		cycle = 0;
	}

	
	if(cycle != 0)
	{
		if(	leds[active_col*8+0] == cycle)
		{
			ROW0_OFF;
		}
		if(	leds[active_col*8+1] == cycle)
		{
			ROW1_OFF;
		}
		if(	leds[active_col*8+2] == cycle)
		{
			ROW2_OFF;
		}
		if(	leds[active_col*8+3] == cycle)
		{
			ROW3_OFF;
		}
		if(	leds[active_col*8+4] == cycle)
		{
			ROW4_OFF;
		}
		if(	leds[active_col*8+5] == cycle)
		{
			ROW5_OFF;
		}
		if(	leds[active_col*8+6] == cycle)
		{
			ROW6_OFF;
		}
		if(	leds[active_col*8+7] == cycle)
		{
			ROW7_OFF;
		}
	

//		ROW0_ON;
		return;
	}
	
	

	ROW0_OFF;
	ROW1_OFF;
	ROW2_OFF;
	ROW3_OFF;
	ROW4_OFF;
	ROW5_OFF;
	ROW6_OFF;
	ROW7_OFF;

	active_col++;
	
	if(active_col == 8)
	{
		active_col = 0;
		if(frame_update > 0)
		{
			frame_update--;
		}
	}
	
	

	switch(active_col)
	{
		case 1:
			COL0_OFF;
			COL1_ON;
		break;
		case 2:
			COL1_OFF;
			COL2_ON;
		break;
		case 3:
			COL2_OFF;
			COL3_ON;
		break;
		case 4:
			COL3_OFF;
			COL4_ON;
		break;
		case 5:
			COL4_OFF;
			COL5_ON;
		break;
		case 6:
			COL5_OFF;
			COL6_ON;
		break;
		case 7:
			COL6_OFF;
			COL7_ON;
		break;
		case 0:
			COL7_OFF;
			COL0_ON;
	}

	for(uint8_t i = 0;i<64;i++)
	{
		leds[i]=leds_buf[i];
	}
                                    
	
	if(	leds[active_col*8+0] != 0)
	{
		ROW0_ON;
	}
	if(	leds[active_col*8+1] != 0)
	{
		ROW1_ON;
	}
	if(	leds[active_col*8+2] != 0)
	{
		ROW2_ON;
	}
	if(	leds[active_col*8+3] != 0)
	{
		ROW3_ON;
	}
	if(	leds[active_col*8+4] != 0)
	{
		ROW4_ON;
	}
	if(	leds[active_col*8+5] != 0)
	{
		ROW5_ON;
	}
	if(	leds[active_col*8+6] != 0)
	{
		ROW6_ON;
	}
	if(	leds[active_col*8+7] != 0)
	{
		ROW7_ON;
	}
	

//	ROW0_ON;
}

ISR (PCINT2_vect)
{
	led_mode++;
	if(led_mode > 9)
	{
		led_mode = 0;
	}
}
                


int main(void)
{
	PORTD |= (1<<PORTD7);
	PCMSK2 |= (1<<PCINT21);
	PCICR = (1<<PCIE2);

	ROW0_DDR |= (1<<ROW0_PIN);
	ROW1_DDR |= (1<<ROW1_PIN);
	ROW2_DDR |= (1<<ROW2_PIN);
	ROW3_DDR |= (1<<ROW3_PIN);
	ROW4_DDR |= (1<<ROW4_PIN);
	ROW5_DDR |= (1<<ROW5_PIN);
	ROW6_DDR |= (1<<ROW6_PIN);
	ROW7_DDR |= (1<<ROW7_PIN);

	COL0_DDR |= (1<<COL0_PIN);
	COL1_DDR |= (1<<COL1_PIN);
	COL2_DDR |= (1<<COL2_PIN);
	COL3_DDR |= (1<<COL3_PIN);
	COL4_DDR |= (1<<COL4_PIN);
	COL5_DDR |= (1<<COL5_PIN);
	COL6_DDR |= (1<<COL6_PIN);
	COL7_DDR |= (1<<COL7_PIN);


	ROW0_OFF;
	ROW1_OFF;
	ROW2_OFF;
	ROW3_OFF;
	ROW4_OFF;
	ROW5_OFF;
	ROW6_OFF;
	ROW7_OFF;

	// only one col at a time !!
	COL0_OFF;
	COL1_OFF;
	COL2_OFF;
	COL3_OFF;
	COL4_OFF;
	COL5_OFF;
	COL6_OFF;
	COL7_OFF;
		

	TCCR0B |= (1<<CS00);
	TIMSK0 |= (1<<TOIE0);
	
	sei();


	uint8_t x=0;	
	uint8_t y=0;	

	uint8_t z=0;
	
	while(1)
	{

		if(led_mode < 2)
		{
			if(frame_update == 0)
			{
/*		
		    for(x = 0; x < 8; x++) {
    	        for(y = 0; y < 8; y++) {
					setLedXY(x, y, rand()&7);
				}
			}*/
			
			tick();
				frame_update=6;
			}
		}
		else if (led_mode < 4)
		{
		    for(x = 0; x < 8; x++) {
    	        for(y = 0; y < 8; y++) {
					setLedXY(x, y, 7);
				}
			}
		}

		else if (led_mode < 6)
		{
		    for(x = 0; x < 8; x++) {
    	        for(y = 0; y < 8; y++) {
					setLedXY(x, y, y);
				}
			}
		}

		else if (led_mode < 8 )
		{
		    for(x = 0; x < 8; x++) {
    	        for(y = 0; y < 8; y++) {
					setLedXY(x, y, x);
				}
			}
		}

		else if (led_mode < 10 )
		{
			if(frame_update == 0)
			{
			    for(x = 0; x < 8; x++) {
    	        	for(y = 0; y < 8; y++) {
						setLedXY(x, y, (z&1)*7);
					}
				}
				z++;
				frame_update=2;
			}
		}

//		_delay_ms(100);
                                    		
	
	}



}


void setLedXY(uint8_t x,uint8_t y, uint8_t brightness)
{
    if((x < 8)&&(y<8)&&(brightness < 8))
	{
        if(brightness == 0) leds_buf[y*8+x] = 0;
		if(brightness == 1) leds_buf[y*8+x] = 1;
		if(brightness == 2) leds_buf[y*8+x] = 2;
		if(brightness == 3) leds_buf[y*8+x] = 4;
		if(brightness == 4) leds_buf[y*8+x] = 8;
		if(brightness == 5) leds_buf[y*8+x] = 20;
		if(brightness == 6) leds_buf[y*8+x] = 40;
		if(brightness == 7) leds_buf[y*8+x] = 70;
	}
}