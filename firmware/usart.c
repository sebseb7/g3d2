#include <avr/io.h>
#include <avr/interrupt.h>

#include "main.h"
#include "usart.h"

#define UART_RXBUFSIZE 250

volatile static uint8_t rxbuf0[UART_RXBUFSIZE];
volatile static uint8_t *volatile rxhead0, *volatile rxtail0;
volatile uint8_t xon = 0;


ISR (USART_RX_vect)//,ISR_NAKED
{
/*	asm volatile("push r24");
	UCSR0B &= ~(1 << RXCIE0);
 	asm volatile("sei");
	asm volatile("push __zero_reg__");
	asm volatile("push r0");
	asm volatile("in r0,__SREG__");
	asm volatile("push r0");
	asm volatile("clr __zero_reg__");
	asm volatile("push r18");
	asm volatile("push r19");
	asm volatile("push r20");
	asm volatile("push r25");
	asm volatile("push r30");
	asm volatile("push r31");*/
	
        int diff;
        uint8_t c;
        c=UDR0;
        diff = rxhead0 - rxtail0;
        if (diff < 0) diff += UART_RXBUFSIZE;
        if (diff < UART_RXBUFSIZE -1)
        {
            *rxhead0 = c;
            ++rxhead0;
            if (rxhead0 == (rxbuf0 + UART_RXBUFSIZE)) rxhead0 = rxbuf0;
            if((diff > 100)&&(xon==0))
			{
				xon=1;
				//set the CTS pin
			}
        }
/*	asm volatile("pop r31");
	asm volatile("pop r30");
	asm volatile("pop r25");
	asm volatile("pop r20");
	asm volatile("pop r19");
	asm volatile("pop r18");
	asm volatile("pop r0");
	asm volatile("out __SREG__,r0");
	asm volatile("pop r0");
	asm volatile("pop __zero_reg__");
	UCSR0B |= (1 << RXCIE0);
	asm volatile("pop r24");
	asm volatile("reti");*/
}


void USART0_Init (void)
{
	// set clock divider
//	#undef BAUD
//	#define BAUD 1000000
//#	#define BAUD 1000000
//	#include <util/setbaud.h>
//	UBRR0H = UBRRH_VALUE;
//	UBRR0L = UBRRL_VALUE;

	UBRR0H = 0;
	UBRR0L = 4;
	
//#if USE_2X
	UCSR0A |= (1 << U2X0);	// enable double speed operation
//#else
//	UCSR0A &= ~(1 << U2X0);	// disable double speed operation
//#endif
	

	// flush receive buffer
	while ( UCSR0A & (1 << RXC0) ) UDR0;

	// set 8N1
	UCSR0C = (1 << UCSZ01) | (1 << UCSZ00);
	UCSR0B &= ~(1 << UCSZ02);



	UCSR0B |= (1 << RXEN0);
	UCSR0B &= ~(1 << TXEN0);
//	UCSR0B |= (1 << RXEN0) | (1 << TXEN0);
	UCSR0B |= (1 << RXCIE0);

	rxhead0 = rxtail0 = rxbuf0;

}



void USART0_putc (char c)
{
	loop_until_bit_is_set(UCSR0A, UDRE0);
	UDR0 = c;
}


uint8_t USART0_Getc_nb(uint8_t *c)
{
    if (rxhead0==rxtail0) return 0;
    *c = *rxtail0;
    if (++rxtail0 == (rxbuf0 + UART_RXBUFSIZE)) rxtail0 = rxbuf0;

    uint8_t diff = rxhead0 - rxtail0;
	if((diff < 10)&&(xon==1))
	{
		xon=0;
		//set the CTS pin
	}

    return 1;
}
