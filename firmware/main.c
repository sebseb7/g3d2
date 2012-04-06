#ifndef F_CPU
#define __OPTIMIZE__
#define F_CPU 20000000
#define __AVR_ATmega88PA__
#endif
#include <inttypes.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdlib.h>


#include "main.h"
#include "pins.h"
#include "usart.h"
#include "addrfont.h"


/* 

	todo:

	- pack two pixel is one byte (on uart)
	- addr selection (select addr with button, save to eeprom)

*/


typedef void (*AppPtr_t)(void) __attribute__ ((noreturn)); 
#ifndef ADDR
#define ADDR 199
//#warning ADDR not defined in Makefile
#endif

uint8_t addr = ADDR;

//these variables are used by the timer intr
uint8_t pixel_step = 0;
uint8_t pixel_step2 = 0;
uint8_t row_step = 0;

uint8_t steps[16] = {1,1,1,1,1,2,3,4,6,7,11,18,30,50,4,1};

uint8_t rowbyte_portc[8] = {~1,~2,~4,~8,~16,~32,~0,~0};
uint8_t rowbyte_portd[8] = {12,12,12,12,12 ,12 ,8,4};

uint8_t colbyte_portb[128]={
							0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
							0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
							0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
							0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
							0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
							0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
							0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
							0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
							};

uint8_t colbyte_portd[128]={
							0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
							0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
							0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
							0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
							0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
							0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
							0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
							0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
							};


uint8_t frame_buffer[32];


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
	
	if(pixel_step == 16)
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

uint8_t addr2 = 0;

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
	
	addr2 = addr;

	if(addr2 > 99)
	{
		addr2 = addr - 100;
/*		if(addr2 == 29)
		{
			addr2 = 35;
		}
*/	}

	setLedAll(0);

	for(uint8_t i = 0;i<36;i++)
	{
		if(i == addr2)
		{
			setLedAll(10);
			_delay_ms(60);
		}
		else
		{
			setLedAll(4);
			_delay_ms(60);
		}
	}

	display_addr();
	
	USART0_Init();

	uint8_t data = 0;  
	uint8_t state = 0;
	uint8_t escape = 0;

	uint8_t idx = 0;

	uint8_t offset_x = (addr2 % 9)*8;
	uint8_t offset_y = 24 - (((addr2 - (addr2 % 9)) / 9) * 8);

    uint8_t pixel_x = 0;
    uint8_t pixel_y = 0;

	uint8_t state_pixel_x = 0;
	uint8_t state_mod_x = 0;
	uint8_t state_pixel_y = 0;
	uint8_t state_mod_y = 0;

	while(1)
	{
		if(USART0_Getc_nb(&data))
		{

			switch(data)
			{
				case 0x42:
					// single pixel
					state = 1;
					idx = 0;
					continue;
				case 0x23:
					// full frame
					state = 2;
	
					state_mod_x = 0;
					state_pixel_x = 0;
					state_pixel_y = 0;
					state_mod_y= 3*9;
					continue;
				case  0x65:
					escape = 1;
					continue;
				case 0x66:
					state = 3;
					continue;
			}
			
			if(escape == 1)
			{
				escape = 0;
				
				switch(data)
				{
					case 0x01:
						data = 0x23;
						break;
					case 0x02:
						data = 0x42;
						break;
					case 0x03:
						data = 0x65;
						break;
					case 0x04:
						data = 0x66;
						break;
				}
			}
			
		
			if(state == 1)
			{
				// wait for our pixel
				switch(idx)
				{
					case 0:
						pixel_x = data;
						break;
					case 1:
						pixel_y = data;
						break;
					case 2:
					
						if((pixel_x == 0)&&(pixel_y == 0)){
							setLedAll(data);
						}else{
							pixel_x--;
							pixel_y--;
							if(
								(pixel_x >= offset_x)&& 
								(pixel_x < (offset_x+8))&& 
								(pixel_y >= offset_y)&&
								(pixel_y < (offset_y+8))
							) 
							{
								setLedXY(7-(pixel_x-offset_x),7-(pixel_y-offset_y),data);
							}
						}
				}
				idx++;
				
			}
			else if(state == 2)
			{

				if(state_mod_x+state_mod_y == addr2)
				{
					frame_buffer[state_pixel_x+(state_pixel_y*4)] = data;
				}

				state_pixel_x++;
				
				if(state_pixel_x==4)
				{
					state_pixel_x = 0;
					state_mod_x++;
					if(state_mod_x == 9)
					{
						state_mod_x = 0;
						state_pixel_y++;
						if(state_pixel_y == 8)
						{
							state_pixel_y=0;
							state_mod_y-=9;
							if(state_mod_y == 247)
							{
								uint8_t ps2 =0;
								for(uint8_t x = 0;x<8;x++)
								{
									for(uint8_t y = 0;y<4;y++)
									{
										setLedXY(7-(y*2),7-x,(frame_buffer[ps2] & 0x0f));
										setLedXY(7-(y*2+1),7-x, ( (frame_buffer[ps2]&0xf0) >> 4 ) );
										ps2++;
			
									}
								}
							}
						}
					}
				}



			}
			else if(state == 3)
			{
				if(data == 0xfd)
				{
//                    UBRR0L = 0;
//                    UCSR0A &= ~(1 << U2X0);
				}
				else if(data == 0xff)
				{
					// get addr
					setLedAll(0);
					setLedXY((addr2/8),(addr2%8),15);
				}
				else if(data == 0xfe)
				{
					// get addr
					
					display_addr();
					
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
/*#ifdef ADDR_100			        
		            UCSR0B &= ~(1 << TXEN0);
#endif		            */
					setLedAll(10);
					for(uint8_t x = 0;x < 8;x++)
					{
						for(uint8_t y = 0;y < 8;y++)
						{
							_delay_ms(0x19);
							setLedXY(x,y,15);
							_delay_ms(0x19);
							setLedXY(x,y,0);
						}
					}
					_delay_ms(0x15);
					setLedAll(0);
				    UCSR0B |= (1 << RXEN0);
			        UCSR0B |= (1 << RXCIE0);
/*#ifdef ADDR_100			        
		            UCSR0B |= (1 << TXEN0);
#endif		            */
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
				colbyte_portb[x*16+i]|=(1<<(y-2));
			}
			else
			{
				colbyte_portb[x*16+i]&=~(1<<(y-2));
			}
		}
	}
	else
	{
		for(uint8_t i = 0;i < 15;i++)
		{
			if(brightness > i)
			{
				colbyte_portd[x*16+i]|=(1<<(y+6));
			}
			else
			{
				colbyte_portd[x*16+i]&=~(1<<(y+6));
			}
		}
	}
}

void setLedAll(uint8_t brightness)
{
	for(uint8_t x = 0;x<8;x++)
	{
		for(uint8_t i = 0;i < 15;i++)
		{
			if(brightness > i)
			{
				colbyte_portb[x*16+i] = 63;
			}
			else
			{
				colbyte_portb[x*16+i] = 0;
			}
		}
		for(uint8_t i = 0;i < 15;i++)
		{
			if(brightness > i)
			{
				colbyte_portd[x*16+i]=192;
			}
			else
			{
				colbyte_portd[x*16+i]=0;
			}
		}
	}
}

void display_addr(void)
{
//	setLedAll(0);
	setLedAll(6);

	for(uint8_t i = 0;i<5;i++)
	{
		for(uint8_t c=0;c<7;c++)
		{
			if((addrfont[i] & (1<<c))==(1<<c))
			{
				if((addr2 % 2) == 1)
				{
					setLedXY(c,7-i,15);
				}
				else
				{
					setLedXY(c,7-i,0);
				}
			}
			else
			{
				setLedXY(c,7-i,6);
			}
		}
	}
}
