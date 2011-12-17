#include <stdlib.h>
#include <stdio.h>


#include "core/LPC17xx.h"
#include "drivers/uart0.h"
#include "tetris.h"
#include "main.h"


volatile uint32_t timeout_ms = 0;
volatile uint32_t timeout_delay_ms = 0;

void SysTick_Handler(void) 
{
	if(timeout_ms > 0) timeout_ms--;
	if(timeout_delay_ms > 0) timeout_delay_ms--;
}

char buffer[72][32];


void delay_ms(uint32_t delay_period_ms)
{
	timeout_delay_ms = delay_period_ms;
	while(timeout_delay_ms > 0);
}


int main(void) 
{
	// don't know why this is incorrect
	SystemCoreClock = 100000000;
	SysTick_Config(SystemCoreClock / 1000);

	timeout_ms=0;

	/* Turn on all of port 1 (this includes the 4 mbed LEDs) */
	LPC_GPIO0->FIODIR |= (1<<19)|(1<<20);
	
	UART0_Init(500000);
	
	// sets SystemCoreClock to 44583722, but why ? 
	SystemCoreClockUpdate();

	for(int y = 0 ; y < 32; y++)
	{	
		for(int x = 0; x < 72;x++)
		{
			buffer[x][y]=10;
			pixel(x,y,0);
		}
	}


	pixel(1,1,10);

	tetris_load();

	pixel(0,0,10);

	while(1)
	{
//		pixel(0,0,10);
		tetris_update();	
		delay_ms(20);
	}

	for(int y = 0 ; y < 32; y++)
	{	
		for(int x = 0; x < 72;x++)
		{
			UART0_Sendchar(104);
			
			int x2 = x % 8;
			int y2 = y % 8;
			int mod = (x-x2)/8 + ((y-y2)/8*9);
			
			UART0_Sendchar(mod);
			UART0_Sendchar(x2);
			UART0_Sendchar(y2);
			UART0_Sendchar(15);

			delay_ms(100);
		}
	}




	while(1)
	{
		LPC_GPIO0->FIOPIN |= (1<<19);
		delay_ms(100);
		LPC_GPIO0->FIOPIN &= ~(1<<19);
		delay_ms(100);

		char buf[1000];

		sprintf(buf,"System Core Clock is set to: %i \n",(int)SystemCoreClock);
		UART0_PrintString(buf);
		sprintf(buf,"timeout is at: %u \n",(unsigned int)timeout_ms);
		UART0_PrintString(buf);

		
		if((LPC_GPIO0->FIOPIN & (1<<10)) == (1<<10))
		{
			LPC_GPIO0->FIOPIN |= (1<<20);
			UART0_PrintString("test1");
		}
		else
		{
			LPC_GPIO0->FIOPIN &= ~(1<<20);
			UART0_PrintString("test2");
			timeout_ms = 10000;
		}
	}
}

int button_down(unsigned int button) {

	return 0;
}



void pixel(int x, int y, unsigned char color) 
{

	if(buffer[x][y] == color)
	{
		return;
	}
	
	buffer[x][y] = color;
	
		
		
			y = 31-y;

			UART0_Sendchar(104);
			
			int x2 = x % 8;
			int y2 = y % 8;
			int mod = (x-x2)/8 + ((y-y2)/8*9);
			
			UART0_Sendchar(mod);
			UART0_Sendchar(x2);
			UART0_Sendchar(y2);
			UART0_Sendchar(color);

}
        
// found it in the internet...
static unsigned int my_rand(void) {
    static unsigned int z1 = 12345, z2 = 12345, z3 = 12345, z4 = 12345;
    unsigned int b;
    b  = ((z1 << 6) ^ z1) >> 13;
    z1 = ((z1 & 4294967294U) << 18) ^ b;
    b  = ((z2 << 2) ^ z2) >> 27;
    z2 = ((z2 & 4294967288U) << 2) ^ b;
    b  = ((z3 << 13) ^ z3) >> 21;
    z3 = ((z3 & 4294967280U) << 7) ^ b;
    b  = ((z4 << 3) ^ z4) >> 12;
    z4 = ((z4 & 4294967168U) << 13) ^ b;
    return (z1 ^ z2 ^ z3 ^ z4);
}

unsigned int rand_int(unsigned int limit) {
    return my_rand() % limit;
}
