#include <inttypes.h>
#include <avr/io.h>
#include <avr/interrupt.h>
//#include <util/delay.h>
#include <stdlib.h>

#include "main.h"
#include "pins.h"
#include "usart.h"


#define DISPLAY_WIDTH 72
#define DISPLAY_HEIGHT 32

typedef void (*AppPtr_t)(void) __attribute__ ((noreturn)); 

uint8_t pixelIsOurs(uint8_t,uint8_t);

uint8_t active_col = 1;

uint8_t addr = 0;
uint8_t module_row = 0;
uint8_t module_column = 0;


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



	USART0_Init();

	
    uint8_t pixel_x = 0;
    uint8_t pixel_y = 0;
    uint8_t pixel_g = 0;
    uint8_t pixel_nr = 0;

    uint8_t frameBuffer[16*3];
    for(uint8_t i = 0;i<(16*3);i++)
    {
        frameBuffer[i]=0;
    }


	uint8_t data = 0;  
	uint8_t state = 0;
	uint8_t escape = 0;
	uint8_t idx = 0;
	uint8_t color_state = 0;
	uint8_t x_state = 0;
	uint8_t y_state = 0;
	setLedXY(1,1,4);


	while(1)
	{
		if(USART0_Getc_nb(&data))
		{

			if(data == 0x42)
			{
				// single pixel
				state = 1;
				idx = 0;
				continue;
			}
			if(data == 0x23)
			{
				// full frame
				state = 2;

				color_state = 0;
				x_state = 0;
				y_state = 0;
				continue;
			}
			if(data == 0x65)
			{
				escape = 1;
				continue;
			}
			if(data == 0x66)
			{
				// bootloader
				state = 3;
				continue;
			}
			if(escape == 1)
			{
				escape = 0;
				if(data == 0x01)
				{
					data = 0x23;
				}
				else if(data == 0x02)
				{
					data = 0x42;
				}
				else if(data == 0x03)
				{
					data = 0x65;
				}
				else if(data == 0x04)
				{
					data = 0x66;
				}
			}
			
		
			if(state == 1)
			{
				// wait for our pixel
				if(idx == 0)
				{
					pixel_x = data;
				}
				if(idx == 1)
				{
					pixel_y = data;
				}
				if(idx == 2)
				{
					pixel_g = data;
/*					if((pixel_x == 0) && (pixel_y == 0))
					{
						setLedXY(0,0,pixel_g);
					}
					else
					{
						pixel_nr = pixelIsOurs(pixel_x,pixel_y);*/
						setLedXY(pixel_x,pixel_y,pixel_g);
/*						if(pixel_nr != 0)
						{
						}
					}*/
				}
				idx++;
				
			}

			if(state == 2)
			{
				// wait for our part of the frame


				pixel_nr = pixelIsOurs(x_state+1,y_state+1);
				if(pixel_nr != 0)
				{
						frameBuffer[((pixel_nr-1)*3)+color_state] = data;
				}
				
				color_state++;
				if(color_state == 3) 
				{
					color_state = 0;
					y_state++;
				}
				if(y_state == DISPLAY_WIDTH)
				{
					y_state=0;
					x_state++;
				}
				if(x_state == DISPLAY_HEIGHT)
				{
//					SetAllLeds(frameBuffer);
				}
			}

			if(state == 3)
			{
				if(data == 0xff)
				{
					// get addr
					// display addr on LEDs
//					SetLed(0,0,0,0);
					for(uint8_t i = 0;i<8;i++)
					{
						if((addr & (1<<i))==(1<<i))
						{
//							SetLed(i+1,0xa0,0,0);
						}
					}
				}
				else if(data == addr)
				{
					// jump to bootloader
					GPIOR2=255;
					AppPtr_t AppStartPtr = (AppPtr_t)0x1800; 
					AppStartPtr();
				}
				else
				{
					//disable UART for a few seconds
			        UCSR0B &= ~(1 << RXCIE0);
				    UCSR0B &= ~(1 << RXEN0);
//					SetLed(0,0,0,150);
					for(uint8_t i = 0;i < 16;i++)
					{
//						_delay_ms(0xbf);
//						SetLed(i+1,0,150,0);
//						writeChannels();
//						_delay_ms(0xbf);
//						SetLed(i+1,150,0,0);
//						writeChannels();
					}
//					_delay_ms(0xbf);
//					SetLed(0,0,0,0);
//					writeChannels();
				    UCSR0B |= (1 << RXEN0);
			        UCSR0B |= (1 << RXCIE0);
					// sleep for bootloader of differend device display progress on LEDs
				}
				state = 0;
			}

		}
	}
}

//returns 0 if that pixel is not on out tile, otherwise LED number (1..16)
uint8_t pixelIsOurs(uint8_t x,uint8_t y)
{
	x--;
	y--;
	
	if( 
		(x >=  module_row      *4) && 
		(x <  (module_row+1)   *4) &&
		(y >=  module_column   *4) && 
		(y <  (module_column+1)*4)
	)
	{
		uint8_t row = x - module_row*4;
		uint8_t col = y - module_column*4;
	
		
	
		return row*4+col+1;
	} 

	return 0;
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
