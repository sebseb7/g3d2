#ifndef MAIN_H_
#define MAIN_H_

#include <stdint.h>
#include "core/LPC17xx.h"


#ifndef NULL
#define NULL ((void *) 0)
#endif


#define set_cs()    (LPC_GPIO2->FIOPIN |=  (1 << 1))
#define clr_cs()    (LPC_GPIO2->FIOPIN &= ~(1 << 1))
#define set_reset() (LPC_GPIO2->FIOPIN |=  (1 << 0))
#define clr_reset() (LPC_GPIO2->FIOPIN &= ~(1 << 0))
#define set_A0()    (LPC_GPIO0->FIOPIN |=  (1 << 6))			// Data
#define clr_A0()    (LPC_GPIO0->FIOPIN &= ~(1 << 6))			// Command
//#define set_scl()   (PORTB |=  (1 << PB7))
//#define clr_scl()   (PORTB &= ~(1 << PB7))
//#define set_si()    (PORTB |=  (1 << PB5))
//#define clr_si()    (PORTB &= ~(1 << PB5))


enum {
	DISPLAY_WIDTH = 72,
	DISPLAY_HEIGHT = 32,
};

enum {
	MODULES_PER_LINE = 9
};

enum {
	BUTTON_UP,
	BUTTON_DOWN,
	BUTTON_LEFT,
	BUTTON_RIGHT,
	BUTTON_A,
	BUTTON_B,
	BUTTON_START,
	BUTTON_SELECT,
};

unsigned int rand_int(unsigned int limit);
void pixel(int x, int y, unsigned char color);

int button_down(unsigned int nr, unsigned int button);
void push_lines(unsigned int nr, unsigned int lines);
int is_occupied(unsigned int nr);



uint16_t crc16(uint8_t * buf, int len);

void ssp1_send_byte (uint8_t buf);
void sspSend(uint8_t portNum, const uint8_t *buf, uint32_t length);
void delay_ms(uint32_t delay_period_ms);

#endif
