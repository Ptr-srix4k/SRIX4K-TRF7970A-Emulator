#define F_CPU 8000000UL 

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "TRF7970A.h"
#include "usart.h"
#include "spi.h"
#include "key.h"

volatile unsigned char BUFF_SPI[20];
volatile unsigned char BUFF_TX[20];

#define DEBUG_HIGH          PORTB |= (1 << PB0);   
#define DEBUG_LOW           PORTB &= ~(1 << PB0); 

//Setup input/output pins
void setup_pins()
{
    //TXD UART as output
    DDRD = (1 << PD1); 
    //Disable input pull-up resistor       
    PORTD = 0;
    
    //PORTB as SPI master
    //PB5 = SCK (output)
    //PB4 = MISO (input)
    //PB3 = MOSI (output)
    //PB2 = SS (output)
    //PB0 = Debug
    DDRB = (1 << PB5) | (1 << PB3) | (1 << PB2) | (1 << PB0);    
    
    SS_HIGH;
    DEBUG_LOW;
}

void setup_interrupt()
{
	//Disable all interrupt
	cli();
	//Interrupt on rising edge
	MCUCR = (1 << ISC01) | (1 << ISC00); 
	//Interrupt on INT0 pin
	GICR = (1 << INT0); 
}

//Read interrupt register
unsigned char TRF7970A_int_read()
{
	unsigned char buff[3];
	
	buff[0] = READ | CONT | IRQ_STATUS;
	buff[1] = 0;
	buff[2] = 0; //Dummy-read 1 byte from register 0x0D
	SPI_tx_rx(buff,3);	
	
	return buff[1];
}

ISR(INT0_vect)
{
	unsigned char int_val;
	unsigned char n_byte;	
	unsigned char command, data_1, data_2, data_3, data_4, data_5;	
	unsigned char n_tx=0;	
	
	//Read interrupt register
	int_val = TRF7970A_int_read();
	
	if ((int_val & 0x40) != 0) //RX data received
	{
	    DEBUG_HIGH
	
		//Read FIFO status register to get the number of byte received
		BUFF_SPI[0] = READ | FIFO_STATUS;
		BUFF_SPI[1] = 0;
		SPI_tx_rx(BUFF_SPI,2);
		n_byte = BUFF_SPI[1];
		
		//Read FIFO bytes
		BUFF_SPI[0] = CONT | READ | FIFO_IO;
		SPI_tx_rx(BUFF_SPI,n_byte + 1);
		
		//Save received command
		command = BUFF_SPI[1];
		data_1  = BUFF_SPI[2];
		data_2  = BUFF_SPI[3];
		data_3  = BUFF_SPI[4];
		data_4  = BUFF_SPI[5];  
		data_5  = BUFF_SPI[6];  
		
		//Reset FIFO
		BUFF_SPI[0] = CMD | RESET_FIFO;
		SPI_tx_rx(BUFF_SPI,1);
		
		//Block receiver
		BUFF_SPI[0] = CMD | BLOCK_RECEIVER;
		SPI_tx_rx(BUFF_SPI,1);
	
		switch (command)
		{			
			//INITIATE
			case 0x06:
				BUFF_SPI[3] = 0x5a; //Fixed chip id	
				_delay_us(18);
				n_tx = 1;	  
				break;
			  
		    //SELECT(CHIP_ID)	  
			case 0x0e:
				BUFF_SPI[3] = data_1;   
				n_tx = 1;	
				break;

			//READ_BLOCK(ADD)
			case 0x08:		
				if (data_1==0xFF)
				{
					BUFF_SPI[6] = (mem_FF       & 255);
					BUFF_SPI[5] = ((mem_FF>>8)  & 255);
					BUFF_SPI[4] = ((mem_FF>>16) & 255);
					BUFF_SPI[3] = ((mem_FF>>24) & 255);
				}
				else
				{
					BUFF_SPI[6] = ( mem[data_1]      & 255);
					BUFF_SPI[5] = ((mem[data_1]>>8)  & 255);
					BUFF_SPI[4] = ((mem[data_1]>>16) & 255);
					BUFF_SPI[3] = ((mem[data_1]>>24) & 255);
				}
				n_tx = 4;		  
				break;
				
			//WRITE_BLOCK(ADD)
			case 0x09:			
				mem[data_1] = (unsigned long int)data_5 + ((unsigned long int)data_4 << 8) + ((unsigned long int)data_3 << 16) + ((unsigned long int)data_2 << 24);
				n_tx = 0;	 
				break;
			
			//GET_UID
			case 0x0B:
				BUFF_SPI[10] = ((UID[0]>>24) & 255);
				BUFF_SPI[ 9] = ((UID[0]>>16) & 255);
				BUFF_SPI[ 8] = ((UID[0]>>8)  & 255);
				BUFF_SPI[ 7] = (UID[0]       & 255);
				BUFF_SPI[ 6] = ((UID[1]>>24) & 255);
				BUFF_SPI[ 5] = ((UID[1]>>16) & 255);
				BUFF_SPI[ 4] = ((UID[1]>>8)  & 255);
				BUFF_SPI[ 3] = (UID[1]       & 255);
				n_tx = 8;	
				break;
			  
			default:
				n_tx = 0;	
				break;	
		}
		
		//Do not transmit anything
		if (n_tx == 0)
		{
			//Enable receiver
			BUFF_SPI[0] = CMD | ENABLE_RECEIVER;
			SPI_tx_rx(BUFF_SPI,1);	
		}
		else
		{	
		    //Write data to TX_LENGTH_1 / TX_LENGTH_2 and FIFO values
		    BUFF_SPI[0] = CONT | WRITE | TX_LENGTH_1;
		    BUFF_SPI[1] = 0;
		    BUFF_SPI[2] = (n_tx << 4);	    
		    SPI_tx_rx(BUFF_SPI,n_tx+3);
			
			//Send transmit command
			BUFF_SPI[0] = CMD | TX_CRC;
			SPI_tx_rx(BUFF_SPI,1);
			
			DEBUG_LOW
		}
	}
	else if ((int_val & 0x80) != 0) //TX finished
	{
	
		//Reset FIFO
		BUFF_SPI[0] = CMD | RESET_FIFO;
		SPI_tx_rx(BUFF_SPI,1);	
	
		//Enable receiver
		BUFF_SPI[0] = CMD | ENABLE_RECEIVER;
		SPI_tx_rx(BUFF_SPI,1);

	}
}

int main(void)
{    
	int i;

	setup_interrupt();
    setup_pins();
    SPI_MasterInit();
    USART_Init(51); //9600Baud
    
    USART_clear_terminal();
    USART_string("SRIX4K Emulator Test");
    
    //Clear buffer
    for(i=0;i<20;i++)
    {
		BUFF_SPI[i] = 0;
	}
    
    //Reset sequence
    BUFF_SPI[0] = CMD | RESET;
    SPI_tx_rx(BUFF_SPI,1);
    BUFF_SPI[0] = CMD | IDLE;
    SPI_tx_rx(BUFF_SPI,1);
    
    _delay_ms(1);
    
    //Reset FIFO
    BUFF_SPI[0] = CMD | RESET_FIFO;
    SPI_tx_rx(BUFF_SPI,1);

    //Read register default just as check
    BUFF_SPI[0] = READ | CONT | CHIP_STATUS;
    SPI_tx_rx(BUFF_SPI,7);

    if ((BUFF_SPI[1] != 0x01) || 
        (BUFF_SPI[2] != 0x21) || 
        (BUFF_SPI[3] != 0x00) || 
        (BUFF_SPI[4] != 0x00) || 
        (BUFF_SPI[5] != 0xC1) ||
        (BUFF_SPI[6] != 0xC1))
    {
		USART_string("Error: Init failed");
		USART_debug_print(BUFF_SPI, 6);
		USART_string("Please reset");
		while(1);
	}

	//Write modulator and SYS_CTRL register
	BUFF_SPI[0] = WRITE | MODULATOR_AND_SYS_CLK_CONTROL;
	BUFF_SPI[1] = 0;
	SPI_tx_rx(BUFF_SPI,2);
		
	//Write NFC Target Detection Level register to zero (workaround)
	BUFF_SPI[0] = WRITE | NFC_TARGET_DETECTION_LEVEL;
	BUFF_SPI[1] = 0;
	SPI_tx_rx(BUFF_SPI,2);

	//Write NFC Target Detection Level register to 0x03
	//RF field level = 11
	BUFF_SPI[0] = WRITE | NFC_TARGET_DETECTION_LEVEL;
	BUFF_SPI[1] = 0x03;
	SPI_tx_rx(BUFF_SPI,2);
	
	//Write ISO control register to 0x25
	//rfid = 1 (card emulation mode)
	//iso_2 = 1 / iso_0 = 1
	BUFF_SPI[0] = WRITE | ISO_CONTROL;
	BUFF_SPI[1] = 0x25;
	SPI_tx_rx(BUFF_SPI,2);
	
	//Write chip status register to 0x20
	//rf_on = 1
	BUFF_SPI[0] = WRITE | CHIP_STATUS;
	BUFF_SPI[1] = 0x20;
	SPI_tx_rx(BUFF_SPI,2);
/*	
	//Change IRQ TX water level registers
	BUFF_SPI[0] = WRITE | FIFO_IRQ_LEVELS;
	BUFF_SPI[1] = 0x03;
	SPI_tx_rx(BUFF_SPI,2);
*/	
	//Change voltage level from automatic to manual
	BUFF_SPI[0] = WRITE | REG_IO_CONTROL;
	BUFF_SPI[1] = 0x07;
	SPI_tx_rx(BUFF_SPI,2);
	
	//Write Interrupt mask register to 0
	BUFF_SPI[0] = WRITE | IRQ_MASK;
	BUFF_SPI[1] = 0x00;
	SPI_tx_rx(BUFF_SPI,2);
	
	//Clear interrupt
	TRF7970A_int_read();
	
	//Enable receiver
	BUFF_SPI[0] = CMD | ENABLE_RECEIVER;
	SPI_tx_rx(BUFF_SPI,1);
	
	USART_string("Init OK");

	//Enable ATMEGA interrupt 
	GIFR = (1 << INTF0); //Clear any possible interrupt flag
	sei();

    while(1);
}
