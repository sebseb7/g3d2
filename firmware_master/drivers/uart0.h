#ifndef UART_H_
#define UART_H_

void UART0_Init(int baudrate);
void UART0_Sendchar(char c);
char UART0_Getchar();
void UART0_PrintString(char *pcString);

#endif /*UART_H_*/
