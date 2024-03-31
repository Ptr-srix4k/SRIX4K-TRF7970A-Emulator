#include <avr/io.h>
#include "spi.h"

void SPI_MasterInit(void)
{
    /* Enable SPI, Master, set clock rate fck/4 */
    SPCR = (1<<SPE)|(1<<MSTR)|(1<<CPHA);
    //SPSR = (1 << SPI2X);
}

void SPI_tx_rx(volatile unsigned char *buff, unsigned char n_data)
{
	int i;

	SS_LOW;

	for(i=0;i<n_data;i++)
    {
		/* Start transmission */
		SPDR = buff[i];
		/* Wait for transmission complete */
		while(!(SPSR & (1<<SPIF)));
        buff[i] = SPDR;
    }
    
    SS_HIGH;
}
