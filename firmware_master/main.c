#include <stdlib.h>
#include <stdio.h>


#include "core/LPC17xx.h"
#include "drivers/uart0.h"


void delay_()
{
	uint16_t a=0;
	uint16_t b=0;
	for(a=0;a<0xFFFc;a++)
	{
		b++;
		b++;
		b++;
		b++;
		b++;
		b++;
		b++;
		b++;
		b++;
		b++;
		b++;
		b++;
		b++;
		b++;
		b++;
		b++;
	}
}



int main(void) {
	/* Your code goes here */

	// don't know why this is incorrect
	SystemCoreClock = 100000000;

	/* Turn on all of port 1 (this includes the 4 mbed LEDs) */
	LPC_GPIO0->FIODIR |= (1<<19)|(1<<20);
	
	UART0_Init(115200);
	
	// sets SystemCoreClock to 44583722, but why ? 
	SystemCoreClockUpdate();

	while(1)
	{
		LPC_GPIO0->FIOPIN |= (1<<19);
		delay_();
		LPC_GPIO0->FIOPIN &= ~(1<<19);
		delay_();


		char buf[100];

		siprintf(buf,"System Core Clock is set to: %i \n",(int)SystemCoreClock);

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
		}
	}
}
