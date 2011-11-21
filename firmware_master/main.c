#include <stdlib.h>
#include <stdio.h>


#include "core/LPC17xx.h"
#include "drivers/uart0.h"


volatile uint32_t timeout_ms = 0;
volatile uint32_t timeout_delay_ms = 0;

void SysTick_Handler(void) 
{
	if(timeout_ms > 0) timeout_ms--;
	if(timeout_delay_ms > 0) timeout_delay_ms--;
}


void delay_ms(uint32_t delay_period_ms)
{
	timeout_delay_ms = delay_period_ms;
	while(timeout_delay_ms > 0);
}


int main(void) {
	/* Your code goes here */
	

	// don't know why this is incorrect
	SystemCoreClock = 100000000;
	SysTick_Config(SystemCoreClock / 1000);

	timeout_ms=0;

	/* Turn on all of port 1 (this includes the 4 mbed LEDs) */
	LPC_GPIO0->FIODIR |= (1<<19)|(1<<20);
	
	UART0_Init(115200);
	
	// sets SystemCoreClock to 44583722, but why ? 
	SystemCoreClockUpdate();

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
