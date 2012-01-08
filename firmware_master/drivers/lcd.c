#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

#include "drivers/ssp.h"
#include "lcd.h"
#include "core/LPC17xx.h"
#include "font8x6.h"
#include "main.h"

#define DISP_W 128
#define DISP_H 64
volatile uint8_t LCD_ORIENTATION=0;

#define DISP_BUFFER ((DISP_H * DISP_W) / 8)

volatile uint8_t display_buffer[DISP_BUFFER];


void lcd_cls (void)
{
	uint16_t i, j;

	for (i = 0; i < DISP_BUFFER; i++)
		display_buffer[i] = 0x00;


	clr_cs ();
	for (i = 0; i < 8; i++)
	{
		clr_A0 ();
		sspSendByte (0xB0 + i);	//1011xxxx
		sspSendByte (0x10);			//00010000
		//		send_byte(0x04);		//00000100 gedreht plus 4 Byte
		sspSendByte(0x00);		//00000000
		//		sspSendByte (LCD_ORIENTATION);	//00000000

		set_A0 ();
		for (j = 0; j < 128; j++)
			sspSendByte (0x00);
	}
	set_cs ();

	//	lcd_xpos = 0;
	//	lcd_ypos = 0;
}


void set_adress (uint16_t adress, uint8_t data)
{
	uint8_t page;
	uint8_t column;

	page = adress >> 7;

	clr_cs ();
	clr_A0 ();
	sspSendByte (0xB0 + page);

	//	column = (adress & 0x7F) + 4; Wenn gedreht
	//	column = (adress & 0x7F);
	column = (adress & 0x7f);// + LCD_ORIENTATION;


	sspSendByte (0x10 + (((column&0xf0) >> 4)&0x0f));
	//	ssp1_send_byte (0x10 + 0);
	sspSendByte (0x00 + (column & 0x0F));
	//	ssp1_send_byte (0x00 + 0);

	set_A0 ();
	sspSendByte (data);


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

void lcdInit()
{
	/* Turn on all of port 1 (this includes the 4 mbed LEDs) */
	LPC_GPIO1->FIODIR |= (1<<25)|(1<<21)|(1<<19);

	//a0
	LPC_GPIO0->FIODIR |= (1<<6);
	//rst_di
	LPC_GPIO2->FIODIR |= (1<<0);
	//cs_di
	LPC_GPIO2->FIODIR |= (1<<1);


	set_cs ();
	clr_reset ();
	delay_ms (10);
	set_reset ();
	set_reset ();

	clr_cs ();
	clr_A0 ();


	sspSendByte (0x40);
	sspSendByte (0xA1);
	sspSendByte (0xC0);
	sspSendByte (0xA6);//A6 normal , A7 reverse
	sspSendByte (0xA2);//Set bias 1/9 (Duty 1/65)


	sspSendByte (0x2F);// power on 
	sspSendByte (0xF8);// boster ratio
	sspSendByte (0x00);// 4x

	sspSendByte (0x27);// v0 voltage regulator
	sspSendByte (0x81);// contrast
	sspSendByte (0x16);// 0x16
	sspSendByte (0xAC);// static indicator
	sspSendByte (0x00);// off
	sspSendByte (0xAF);// display on / 0xAE == off

	set_cs ();


	lcd_cls ();

}
