#include <avr/io.h>
#include <stdio.h>
#include "usart.h"

void USART_Init(unsigned int baud )
{
    /* Set baud rate */
    UBRRH = (unsigned char)(baud>>8);
    UBRRL = (unsigned char)baud;
    /* Enable transmitter*/
    UCSRB = (1<<TXEN);
    /* Set frame format: 8data, 1stop bit */
    UCSRC = (1<<URSEL)|(3<<UCSZ0);
}

unsigned char USART_Receive( void )
{
    /* Wait for data to be received */
    while ( !(UCSRA & (1<<RXC)) );
    /* Get and return received data from buffer */
    return UDR;
}

void USART_Transmit(unsigned char data )
{
    /* Wait for empty transmit buffer */
    while ( !( UCSRA & (1<<UDRE)) );
    /* Put data into buffer, sends the data */
    UDR = data;
}

void USART_Flush( void )
{
    unsigned char dummy;
    while ( UCSRA & (1<<RXC) ) dummy = UDR;
}

void USART_string(char *data)
{
	int i=0;
	
	do
	{
		USART_Transmit(data[i]);
		i++;
	}while(data[i] != '\0');
	
	//CR + LF
	USART_Transmit(0x0a);
	USART_Transmit(0x0d);
}

void USART_clear_terminal()
{
	USART_Transmit(0x1b);
    USART_string("[2J");
	USART_Transmit(0x1b);
    USART_string("[H"); 
}

void USART_debug_print(volatile unsigned char *buff, unsigned char num)
{
    int i;
    char str[80];
    
    for(i=1;i<(num+1);i++)
    {
        sprintf(str, "BUFF_SPI[%d] = %x", i,buff[i]);
        USART_string(str);
    }
}
