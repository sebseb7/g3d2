#ifndef LCD_H_
#define LCD_H_

#define set_cs()    (LPC_GPIO2->FIOPIN |=  (1 << 1))
#define clr_cs()    (LPC_GPIO2->FIOPIN &= ~(1 << 1))
#define set_reset() (LPC_GPIO2->FIOPIN |=  (1 << 0))
#define clr_reset() (LPC_GPIO2->FIOPIN &= ~(1 << 0))
#define set_A0()    (LPC_GPIO0->FIOPIN |=  (1 << 6))                   // Data
#define clr_A0()    (LPC_GPIO0->FIOPIN &= ~(1 << 6))                   // Command
//#define set_scl()   (PORTB |=  (1 << PB7))
//#define clr_scl()   (PORTB &= ~(1 << PB7))
//#define set_si()    (PORTB |=  (1 << PB5))
//#define clr_si()    (PORTB &= ~(1 << PB5))

void lcd_plot (uint8_t xpos, uint8_t ypos, uint8_t mode);
void lcdInit(void);


#endif
