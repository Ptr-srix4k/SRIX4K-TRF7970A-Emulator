#define F_CPU 8000000UL 

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <avr/eeprom.h>

#define INIT                0
#define NUM_DATA            1
#define RX_DATA             2
#define TX_DATA             3

#define CS_HIGH             PORTC |= (1 << PC0);   
#define CS_LOW              PORTC &= ~(1 << PC0); 
#define SCK_HIGH            PORTC |= (1 << PC1);   
#define SCK_LOW             PORTC &= ~(1 << PC1); 
#define SDI_HIGH            PORTC |= (1 << PC2);   
#define SDI_LOW             PORTC &= ~(1 << PC2); 
#define SDO_READ            (PINC & (1 << PC3))

unsigned char   BUFF_SPI[128];
unsigned char   BUFF_TX[128];

void USART_Init(unsigned int baud )
{
    /* Set baud rate */
    UBRRH = (unsigned char)(baud>>8);
    UBRRL = (unsigned char)baud;
    /* Enable receiver*/
    UCSRB = (1<<RXEN)|(1<<TXEN);
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

//Setup input/output pins
void setup_pins()
{
    //TXD UART as output
    DDRD = (1 << PD1); 
    //Disable input pull-up resistor       
    PORTD = 0;
    //Set PB0 as input
    DDRB = 0;    
    
    //PORTC as SPI output
    //PC0 = CS output
    //PC1 = SCK output
    //PC2 = SDI output
    //PC3 = SDO input
    DDRC = (1 << PC0) | (1 << PC1) | (1 << PC2);
    PORTC = 0;
}

int main(void)
{    
    unsigned char   state = INIT;
    unsigned char   next  = INIT;
    unsigned char   spi_rd = 0;
    unsigned char   n_data = 0;
    int i = 0;
    int j = 0;

    setup_pins();
    USART_Init(51); //9600Baud
    
    while(1)
    {
        switch (state)
        {
            case INIT:
            
            //Reset SPI buffers
            for(i=0;i<128;i++)
            {
                BUFF_SPI[i] = 0;
                BUFF_TX[i] = 0;
            }
            
            //Set SPI pins
            CS_HIGH;
            SCK_LOW;
            SDI_LOW;
            
            USART_Flush();
            
            next = NUM_DATA;
            
            break;

            //Wait to receive the number of bytes to transmit
            case NUM_DATA:
            
                n_data = USART_Receive();
                next = RX_DATA;
            
            break;
            
            //Wait to receive the bytes to transmit
            case RX_DATA:
            
                for(i=0;i<n_data;i++)
                    BUFF_SPI[i] = USART_Receive();   
                    
                next = TX_DATA;
            
            break;
            
            case TX_DATA:
            
                CS_LOW;
                
                //Tx all the data
                for(i=0;i<n_data;i++)
                {
                    
                    for(j=7;j>=0;j--)
                    {
                        if ((BUFF_SPI[i] & (unsigned char)(1 << j)) == 0)
                        {
                            SDI_LOW;
                        }
                        else
                        {
                            SDI_HIGH;
                        }
                        
                        _delay_us(1);    
                        SCK_HIGH;
                        _delay_us(1);
                        SCK_LOW;
                        
                        //Readback data on SDO
                        BUFF_SPI[i] &= (unsigned char)(~(1 << j));
                        spi_rd = (SDO_READ == 0) ? 0 : 1;
                        BUFF_SPI[i] |= (spi_rd << j);
                    }
                }
                
                CS_HIGH;
                
                //Tx back received data
                UCSRB = (1<<TXEN); //Enable UART transmitter
                for(i=0;i<n_data;i++)
                    USART_Transmit(BUFF_SPI[i]);
                
                UCSRB = 0; //Clear buffers
                
                while ((PINB & (1 << PINB0)) == 0);  //Wait DTR to be one again
                
                //Reset SPI TX buffer
                for(i=0;i<128;i++)
                    BUFF_TX[i] = 0;
                                           
                UCSRB = (1<<RXEN)|(1<<TXEN); //Enable UART              
                n_data = 0;
                next = NUM_DATA;                   
            
            break;
            
            default: 
                
            break;
        }       
        state = next;
    }
}
