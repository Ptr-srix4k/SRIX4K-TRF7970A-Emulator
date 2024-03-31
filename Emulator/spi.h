#ifndef SPI_H
#define SPI_H

#define SS_HIGH             PORTB |= (1 << PB2);   
#define SS_LOW              PORTB &= ~(1 << PB2); 

void SPI_MasterInit(void);
void SPI_tx_rx(volatile unsigned char *BUFF_SPI, unsigned char n_data);

#endif
