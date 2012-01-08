#include <stdlib.h>
#include <stdio.h>


#include "drivers/uart0.h"
#include "drivers/nrf24l01p.h"
#include "drivers/lcd.h"
#include "drivers/ssp.h"
#include "tetris.h"
#include "main.h"


volatile uint32_t timeout_ms = 0;
volatile uint32_t timeout_delay_ms = 0;



#define CHANNEL 81
#define MAC     "\x52\x45\x4D\x30\x54"



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
	
	UART0_Init(115200);


	sspInit();
	lcdInit();


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

