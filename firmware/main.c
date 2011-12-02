#include <inttypes.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdlib.h>

#include "main.h"
#include "pins.h"
#include "usart.h"


/* 

	todo:

	- pack one line in three bytes
	- module shift
	- addr selection (select addr with button, save to eeprom)

*/

#define DISPLAY_WIDTH 72
#define DISPLAY_HEIGHT 32

typedef void (*AppPtr_t)(void) __attribute__ ((noreturn)); 

uint8_t pixelIsOurs(uint8_t,uint8_t);

uint8_t addr = 1;

//these variabled are used by the timer intr
uint8_t pixel_step = 0;
uint8_t pixel_step2 = 0;
uint8_t row_step = 0;

uint8_t steps[15] = {1,1,1,1,1,2,3,4,6,7,11,18,30,50,1};

uint8_t rowbyte_portc[8] = {~1,~2,~4,~8,~16,~32,~0,~0};
uint8_t rowbyte_portd[8] = {12,12,12,12,12 ,12 ,8,4};

uint8_t bufferfree = 1;

uint8_t colbyte_portb[120]={
							0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
							0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
							0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
							0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
							0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
							0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
							0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
							0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
							};

uint8_t colbyte_portd[120]={
							0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
							0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
							0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
							0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
							0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
							0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
							0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
							0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
							};

/*uint8_t colbyte_portb1[56]={
							0,0,0,0,0,0,0,
							0,0,0,0,0,0,0,
							0,0,0,0,0,0,0,
							0,0,0,0,0,0,0,
							0,0,0,0,0,0,0,
							0,0,0,0,0,0,0,
							0,0,0,0,0,0,0,
							0,0,0,0,0,0,0,
							};

uint8_t colbyte_portd1[56]={
							0,0,0,0,0,0,0,
							0,0,0,0,0,0,0,
							0,0,0,0,0,0,0,
							0,0,0,0,0,0,0,
							0,0,0,0,0,0,0,
							0,0,0,0,0,0,0,
							0,0,0,0,0,0,0,
							0,0,0,0,0,0,0,
							};
*/


ISR (TIMER1_OVF_vect)
{
	OCR1A = 0x50*steps[pixel_step];


	uint8_t y = PORTD;

	if (pixel_step == 0)
	{
		PORTC = 0xff;
		y |= ((1<<PORTD2)|(1<<PORTD3));
		PORTD = y;
	}


	PORTB = colbyte_portb[pixel_step2];
	y &= ~((1<<PORTD6)|(1<<PORTD7));
	y |= colbyte_portd[pixel_step2];
	PORTD = y;

	if (pixel_step == 0)
	{
		PORTC = rowbyte_portc[row_step];
		y &= ~((1<<PORTD2)|(1<<PORTD3));
		y |= rowbyte_portd[row_step];
		PORTD = y;
	}

	
	pixel_step++;
	pixel_step2++;
	
	if(pixel_step == 15)
	{
		pixel_step = 0;
		row_step++;
		if(row_step==8)
		{
			row_step=0;
			pixel_step2 = 0;
			
			if(bufferfree == 1)
			{
			
/*				for(uint8_t i=0;i<56;i++)
				{
					colbyte_portd[i]=colbyte_portd1[i];
					colbyte_portb[i]=colbyte_portb1[i];
				}
*/			}
		}
	}
}


int main(void)
{
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
		

	TCCR1A |= (1<<WGM10)|(1<<WGM11);
	TCCR1B |= (1<<WGM12)|(1<<WGM13)|(1<<CS10);
	TIMSK1 |= (1<<TOIE1);
	OCR1A = 0x50;
	
	DDRD |= (1<<PORTD5);
	
	sei();


	USART0_Init();

	
    uint8_t pixel_x = 0;
    uint8_t pixel_y = 0;

	uint8_t data = 0;  
	uint8_t state = 0;
	uint8_t escape = 0;
	uint8_t idx = 0;
	uint8_t x_state = 0;
	uint8_t y_state = 0;
	uint8_t mod_state = 0;
//	setLedXY(1,1,1);

//	setLedXY(0,0,7);


	while(1)
	{
		if(USART0_Getc_nb(&data))
//		if( UCSR0A & (1<<RXC0) )
		{
//			data = UDR0;


			if(data == 0x42)
			{
				// single pixel
				state = 1;
				idx = 0;
				continue;
			}
			else if(data == 0x23)
			{
				// full frame
				state = 2;

//				bufferfree = 0;

				x_state = 0;
				y_state = 0;
				mod_state = 0;
				continue;
			}
			else if(data == 0x65)
			{
				escape = 1;
				continue;
			}
			else if(data == 0x66)
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
				else if(idx == 1)
				{
					pixel_y = data;
				}
				else if(idx == 2)
				{
					setLedXY(pixel_x,pixel_y,data);
				}
				idx++;
				
			}
			else if(state == 2)
			{

				if(mod_state == addr)
				{
					setLedXY(x_state,y_state,data);
				}
				else
				{
//					bufferfree = 1;
				}
				

				y_state++;

				if(y_state == 8)
				{
					y_state=0;
					x_state++;
					if(x_state == 8)
					{
						x_state=0;
						mod_state++;
					}
				}
			}
			else if(state == 3)
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


void setLedXY(uint8_t x,uint8_t y, uint8_t brightness)
{
	if(y > 1)
	{
		for(uint8_t i = 0;i < 15;i++)
		{
			if(brightness > i)
			{
				colbyte_portb[x*15+i]|=(1<<(y-2));
			}
			else
			{
				colbyte_portb[x*15+i]&=~(1<<(y-2));
			}
		}
	}
	else
	{
		for(uint8_t i = 0;i < 15;i++)
		{
			if(brightness > i)
			{
				colbyte_portd[x*15+i]|=(1<<(y+6));
			}
			else
			{
				colbyte_portd[x*15+i]&=~(1<<(y+6));
			}
		}
	}
}
