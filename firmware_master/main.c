#include <stdlib.h>
#include <stdio.h>


#include "drivers/uart0.h"
#include "drivers/nrf24l01p.h"
#include "tetris.h"
#include "main.h"
#include "font8x6.h"


volatile uint32_t timeout_ms = 0;
volatile uint32_t timeout_delay_ms = 0;

#define DISP_W 128
#define DISP_H 64
volatile uint8_t LCD_ORIENTATION=0;

#define DISP_BUFFER ((DISP_H * DISP_W) / 8)

volatile uint8_t display_buffer[DISP_BUFFER];

#define CHANNEL 81
#define MAC     "\x52\x45\x4D\x30\x54"


    

void sspSend (uint8_t portNum, const uint8_t *buf, uint32_t length)
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

void ssp1_send_byte (uint8_t buf)
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


void sspReceive(uint8_t portNum, uint8_t *buf, uint32_t length)
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


void sspSendReceive(uint8_t portNum, uint8_t *buf, uint32_t length)
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

void lcd_cls (void)
{
	uint16_t i, j;

//	memset (display_buffer, 0, 1024);


	for (i = 0; i < DISP_BUFFER; i++)
		display_buffer[i] = 0x00;


	clr_cs ();
	for (i = 0; i < 8; i++)
	{
		clr_A0 ();
		ssp1_send_byte (0xB0 + i);	//1011xxxx
		ssp1_send_byte (0x10);			//00010000
//		send_byte(0x04);		//00000100 gedreht plus 4 Byte
		ssp1_send_byte(0x00);		//00000000
//		ssp1_send_byte (LCD_ORIENTATION);	//00000000

		set_A0 ();
		for (j = 0; j < 128; j++)
			ssp1_send_byte (0x00);
	}
	set_cs ();

//	lcd_xpos = 0;
//	lcd_ypos = 0;
}


void set_adress (uint16_t adress, uint8_t data)
{
//	return;


	uint8_t page;
	uint8_t column;

	page = adress >> 7;

	//UART0_Sendchar(page);

	clr_cs ();

	clr_A0 ();
	ssp1_send_byte (0xB0 + page);
//	ssp1_send_byte (0xB0 + 0);

//	column = (adress & 0x7F) + 4; Wenn gedreht
//	column = (adress & 0x7F);
	column = (adress & 0x7f);// + LCD_ORIENTATION;


	ssp1_send_byte (0x10 + (((column&0xf0) >> 4)&0x0f));
//	ssp1_send_byte (0x10 + 0);
	ssp1_send_byte (0x00 + (column & 0x0F));
//	ssp1_send_byte (0x00 + 0);

	set_A0 ();
	ssp1_send_byte (data);


	set_cs ();

}

void lcd_plot (uint8_t xpos, uint8_t ypos, uint8_t mode)
{
	uint16_t adress;
	uint8_t mask;

	if ((xpos < DISP_W) && (ypos < DISP_H))
	{
		adress = (ypos / 8) * DISP_W + xpos;		// adress = 0/8 * 128 + 0   = 0
		mask = 1 << (ypos & 0x07);					// mask = 1<<0 = 1

		adress &= DISP_BUFFER - 1;

		switch (mode)
		{
			case 0:
				display_buffer[adress] &= ~mask;
				break;
			case 1:
				display_buffer[adress] |= mask;
				break;
			case 2:
				display_buffer[adress] ^= mask;
				break;
		}

		set_adress (adress, display_buffer[adress]);
	}
}

void lcd_putc (uint8_t x, uint8_t y, uint8_t c, uint8_t mode)
{
	uint8_t ch;
	uint8_t i;
	uint16_t adress;


	c &= 0x7f;
	
	adress = y * 128 + x * 6;
	adress &= 0x3FF;
		
	for (i = 0; i < 6; i++)
	{
		ch = font8x6[c][i];

		switch (mode)
		{
			case 0:
				display_buffer[adress+i] = ch;
				break;
			case 1:
				display_buffer[adress+i] |= ch;
				break;
			case 2:
				display_buffer[adress+i] ^= ch;
				break;
			case 3:
				display_buffer[adress+i] &= ch;
				break;
			case 4:
				display_buffer[adress+i] &= ~ch;
				break;
		}
		
		set_adress (adress + i, display_buffer[adress + i]);
	}
}


void SysTick_Handler(void) 
{
	if(timeout_ms > 0) timeout_ms--;
	if(timeout_delay_ms > 0) timeout_delay_ms--;
}

char buffer[DISPLAY_WIDTH][DISPLAY_HEIGHT];


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
	LPC_GPIO1->FIODIR |= (1<<25)|(1<<21)|(1<<19);

	//a0
	LPC_GPIO0->FIODIR |= (1<<6);
	//rst_di
	LPC_GPIO2->FIODIR |= (1<<0);
	//cs_di
	LPC_GPIO2->FIODIR |= (1<<1);



	
//	UART0_Init(500000);
	UART0_Init(115200);

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
	
	set_cs ();
	clr_reset ();
	delay_ms (10);
	set_reset ();
	set_reset ();





	clr_cs ();
	clr_A0 ();


	ssp1_send_byte (0x40);
	ssp1_send_byte (0xA1);
	ssp1_send_byte (0xC0);
	ssp1_send_byte (0xA6);//A6 normal , A7 reverse
	ssp1_send_byte (0xA2);//Set bias 1/9 (Duty 1/65)


	ssp1_send_byte (0x2F);// power on 
	ssp1_send_byte (0xF8);// boster ratio
	ssp1_send_byte (0x00);// 4x

	ssp1_send_byte (0x27);// v0 voltage regulator
	ssp1_send_byte (0x81);// contrast
	ssp1_send_byte (0x16);// 0x16
	ssp1_send_byte (0xAC);// static indicator
	ssp1_send_byte (0x00);// off
	ssp1_send_byte (0xAF);// display on / 0xAE == off

	set_cs ();


	lcd_cls ();



	// sets SystemCoreClock to 44583722, but why ? 
	SystemCoreClockUpdate();

	for(int y = 0 ; y < DISPLAY_HEIGHT; y++)
	{	
		for(int x = 0; x < DISPLAY_WIDTH;x++)
		{
			buffer[x][y]=10;
			pixel(x,y,0);
		}
	}

	//ce
	LPC_GPIO2->FIODIR |= (1<<2);
	//cs_rad
	LPC_GPIO2->FIODIR |= (1<<6);

//	nrf_init();
//	nrf_config_set(&config);


	tetris_load();
		nrf_init();
		delay_ms(100);
	struct NRF_CFG config = {
    	.channel= CHANNEL,
	    .txmac= MAC,
	    .nrmacs=1,
	    .mac0=  MAC,
	    .maclen ="\x20",
	};
		nrf_config_set(&config);
		delay_ms(50);


		uint8_t serialmsg_message[32] = { 0x20,0x47,0x41,0x00,0x00,0x00,0x00,0x05,0x00,0x00,0x00,0xfc,0x07,0x36,0xc6,0x47,0x53,0x6e,0x07,0x00,0x05,0x10,0x54,0x65,0x74,0x72,0x69,0x73,0x56,0x49,0x61,0x2f };
		delay_ms(50);

	while(1)
	{


		nrf_rcv_pkt_end();
		snd_pkt_no_crc(32, serialmsg_message);
		nrf_rcv_pkt_start();

//		delay_ms(500);

		timeout_ms = 20;

		tetris_update();
	    
		
		int len;
        uint8_t buf[32];
        len=nrf_rcv_pkt_poll(sizeof(buf),buf);
        if( len > 0 ){
//            puts("\\1");
//            dump_encoded(len, buf);
//            puts("\\0");
        }	



		if(timeout_ms > 0)
			delay_ms(timeout_ms);
	}

}

int button_down(unsigned int nr,unsigned int button) {
	return 0;
}

void pixel(int x, int y, unsigned char color) 
{
	if(buffer[x][y] == color)
	{
		return;
	}

	
	uint8_t col2 = 0;
	
	if(color > 2)
	{
		col2 = 1;
	}

	lcd_plot (x, y, col2);
	buffer[x][y] = color;
	

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

int is_occupied(unsigned int nr) {
    return 0;
}
void push_lines(unsigned int nr, unsigned int lines)
{
}
//
// x,y = character-Pos. !
//
// mode: 0=Overwrite, 1 = OR, 2 = XOR, 3 = AND, 4 = Delete

