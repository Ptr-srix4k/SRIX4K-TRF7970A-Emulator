#ifndef USART_H
#define USART_H

void USART_Init(unsigned int baud );
unsigned char USART_Receive( void );
void USART_Transmit(unsigned char data );
void USART_Flush( void );
void USART_string(char *data);
void USART_clear_terminal();
void USART_debug_print(volatile unsigned char *buff, unsigned char num);

#endif

