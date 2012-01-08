#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

#include "core/LPC17xx.h"

void sspSend (const uint8_t *buf, uint32_t length)
{
  uint32_t i;
  uint8_t Dummy = Dummy;

    for (i = 0; i < length; i++)
    {
      /* Move on only if NOT busy and TX FIFO not full. */
      while ((LPC_SSP1->SR & (1<<1 | 1<<4)) != (1<<1));
      LPC_SSP1->DR = *buf;
      buf++;
		// buy & rne
      while ( (LPC_SSP1->SR & ((1<<4)|(1<<2))) != (1<<2) );
      /* Whenever a byte is written, MISO FIFO counter increments, Clear FIFO
      on MISO. Otherwise, when SSP0Receive() is called, previous data byte
      is left in the FIFO. */
      Dummy = LPC_SSP1->DR;
    }

  return;
}

void sspSendByte(uint8_t buf)
{
	uint8_t Dummy = Dummy;

	/* Move on only if NOT busy and TX FIFO not full. */
	while ((LPC_SSP1->SR & (1<<1 | 1<<4)) != (1<<1));
      LPC_SSP1->DR = buf;

		// buy & rne
      while ( (LPC_SSP1->SR & ((1<<4)|(1<<2))) != (1<<2) );
      /* Whenever a byte is written, MISO FIFO counter increments, Clear FIFO
      on MISO. Otherwise, when SSP0Receive() is called, previous data byte
      is left in the FIFO. */
      Dummy = LPC_SSP1->DR;

  return;
}


void sspReceive(uint8_t *buf, uint32_t length)
{
  uint32_t i;

    for ( i = 0; i < length; i++ )
    {
      /* As long as the receive FIFO is not empty, data can be received. */
      LPC_SSP1->DR = 0xFF;
  
      /* Wait until the Busy bit is cleared */
      while ( (LPC_SSP1->SR & (1<<4|1<<2)) != (1<<2));
      
      *buf = LPC_SSP1->DR;
      buf++;
    }

  return; 
}


void sspSendReceive(uint8_t *buf, uint32_t length)
{
  uint32_t i;
  uint8_t Dummy = Dummy;

    for (i = 0; i < length; i++)
    {
      /* Move on only if NOT busy and TX FIFO not full. */
      while ((LPC_SSP1->SR & (1<<1 | 1<<4)) != (1<<1));
      LPC_SSP1->DR = *buf;
  
      while ( (LPC_SSP1->SR & (1<<4|1<<2)) != (1<<2));
      /* Whenever a byte is written, MISO FIFO counter increments, Clear FIFO 
      on MISO. Otherwise, when SSP0Receive() is called, previous data byte
      is left in the FIFO. */
      *buf = LPC_SSP1->DR;
      buf++;
    }

  return; 
}

void sspInit()
{
    LPC_PINCON->PINSEL0 &= ~((1<<14)|(1<<15)|(1<<19)|(1<<18));
    LPC_PINCON->PINSEL0 |= ((1 << 15) | (1 << 19));

	LPC_SC->PCONP        |= (1 << 10);              /* Enable power to PCSSP1 block  */
  
  
  	//looks like the requency is 12.5Mhz
    LPC_SC->PCLKSEL0 &= ~(3<<20);
    LPC_SC->PCLKSEL0 |=  (2<<20);
      
      
    // 25 Mhz ?  
    //LPC_SSP1->CPSR = 3;

	// 5 Mhz
    LPC_SSP1->CPSR = 10;
        
    //LPC_SSP1->IMSC = 0x0006;
          
    LPC_SSP1->CR0  = 0x0007;                        /* 8Bit, CPOL=0, CPHA=0         */
	LPC_SSP1->CR1  = 0x0002;  

}
