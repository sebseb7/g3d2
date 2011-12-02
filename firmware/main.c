#include <inttypes.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdlib.h>

#include "main.h"
#include "pins.h"
#include "usart.h"


/* 

	new new idea:
	
	- 3 ports == 3 bytes a 64 states (2 bytes setzen, 1 bytes verunden)
	- 64 dynamische interrupts ueber 16bit timer mit variabler laufzeit.
	
	- doppelte (dreifache?)anzahl buffers (in einen buffer schreiben die uats, aus dem anderen lesen die ints)
	- nach je 64 zyklen umschalten

	- uat interrupt: disable uart int bit, renable global interrupt, at the end of uart , reenable uart
	- check wiht oszi
	- try to skip full frame bytes in uart int
	- other type of ringbuffer ? check uart int runtime agains bytes in p


70 40 20 8 4 2 1

	new idea:
	
	cycle0: active all full on LEDs
	cycle10: copy buffer
	cycle20: activate all 6/7 leds
	cycle40: activate all 5/7 leds
	cycle62: activate all 4/7 leds
	cycle66: activate all 3/7 leds
	cycle68: activate all 2/7 leds
	cycle69: activate all 1/7 leds
	cycle70: all OUT
	cycle71: cycle = 0
*/


#define DISPLAY_WIDTH 72
#define DISPLAY_HEIGHT 32

typedef void (*AppPtr_t)(void) __attribute__ ((noreturn)); 

uint8_t pixelIsOurs(uint8_t,uint8_t);



uint8_t addr = 0;
uint8_t module_row = 0;
uint8_t module_column = 0;



//these variabled are used by the timer intr
uint8_t pixel_step = 0;
uint8_t pixel_step2 = 0;
uint8_t row_step = 0;

uint8_t steps[7] = {1,2,4,12,20,30,1};

uint8_t rowbyte_portc[8] = {~1,~2,~4,~8,~16,~32,~0,~0};
uint8_t rowbyte_portd[8] = {12,12,12,12,12 ,12 ,8,4};

uint8_t colbyte_portb[56]={
							0,0,0,0,0,0,0,
							0,0,0,0,0,0,0,
							0,0,0,0,0,0,0,
							0,0,0,0,0,0,0,
							0,0,0,0,0,0,0,
							0,0,0,0,0,0,0,
							0,0,0,0,0,0,0,
							0,0,0,0,0,0,0,
							};

uint8_t colbyte_portd[56]={
							0,0,0,0,0,0,0,
							0,0,0,0,0,0,0,
							0,0,0,0,0,0,0,
							0,0,0,0,0,0,0,
							0,0,0,0,0,0,0,
							0,0,0,0,0,0,0,
							0,0,0,0,0,0,0,
							0,0,0,0,0,0,0,
							};



ISR (TIMER1_OVF_vect)
{
	OCR1A = 0xff*steps[pixel_step];


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
	
	if(pixel_step == 7)
	{
		pixel_step = 0;
		row_step++;
		if(row_step==8)
		{
			row_step=0;
			pixel_step2 = 0;
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
	OCR1A = 0xff;
	

	
	DDRD |= (1<<PORTD5);
	

	addr = 0;
	module_column = addr % (DISPLAY_WIDTH/8);
	module_row    = (addr - module_column) / (DISPLAY_HEIGHT/8);
	
	
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
			PORTD |= (1<<PORTD5);
//			data = UDR0;


			if(data == 0x42)
			{
				// single pixel
				state = 1;
				idx = 0;
		PORTD &= ~(1<<PORTD5);
				continue;
			}
			else if(data == 0x23)
			{
				// full frame
				state = 2;

				x_state = 0;
				y_state = 0;
				mod_state = 0;
		PORTD &= ~(1<<PORTD5);
				continue;
			}
			else if(data == 0x65)
			{
				escape = 1;
		PORTD &= ~(1<<PORTD5);
				continue;
			}
			else if(data == 0x66)
			{
				// bootloader
				state = 3;
		PORTD &= ~(1<<PORTD5);
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

				if(mod_state == 0)
				{
					setLedXY(x_state,y_state,data);
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
		PORTD &= ~(1<<PORTD5);
		}
	}
}


void setLedXY(uint8_t x,uint8_t y, uint8_t brightness)
{
	if(y > 1)
	{
		for(uint8_t i = 0;i < 7;i++)
		{
			if(brightness > i)
			{
				colbyte_portb[x*7+i]|=(1<<(y-2));
			}
			else
			{
				colbyte_portb[x*7+i]&=~(1<<(y-2));
			}
		}
	}
	else
	{
		for(uint8_t i = 0;i < 7;i++)
		{
			if(brightness > i)
			{
				colbyte_portd[x*7+i]|=(1<<(y+6));
			}
			else
			{
				colbyte_portd[x*7+i]&=~(1<<(y+6));
			}
		}
	}
}
