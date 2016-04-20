/* SPI Routines for the ATMEGA Micro
 * Author: Adrian Gin
 * Date: 27/06/07
 *
 */
 
 
#include <avr/io.h>
#include <avr/interrupt.h>
#include "spi.h"

volatile uint8_t spiTransferComplete = TRANSFER_COMPLETE;

ISR(SPI_STC_vect)
{
   cli();
   
   spiTransferComplete = TRANSFER_COMPLETE;
   
   sei();
}

/** Sets the micro up as a
 * SPI Master
 * MOSI configured as an output,
 * MISO as an input,
 * nSS as an output,
 * CLK as an output 
 */
void SPI_Init(void)
{
   
   SPI_PORT |= (1 << nSS);
   /* Setup ports */
   /* Initiate as Master and Use CPHA = 1*/
   SPCR |= ((1 << SPE) | (1 << MSTR) | (1 << CPHA) );
    
   /* Setup ports */
   SPI_DDR |= ( (1 << SCK) | (1 << nSS) | (1 << MOSI) );
   SPI_DDR &=  ~(1 << MISO);
   SPI_PORT &= ~((1 << MOSI));
   SPI_PORT |= ((1 << nSS) | (1 << MISO) | (1 << SCK) | (1 << MOSI));
     

   SPCR |= ((1 << SPR0) | (1 << SPR1));

}




uint8_t SPI_TxByte(uint8_t data)
{
   //while( !spiTransferComplete );
   SPDR = data;
   //spiTransferComplete = TRANSFER_INCOMPLETE;
   while( ! ( SPSR & (1 << SPIF)));
   SPSR &= ~(1 << SPIF);
   
   return SPDR;
}


/* Transmits the first 'n' bytes of 'data' along the SPI bus
 * Return values are ignored. */
void SPI_TxBlock(uint8_t* data, uint8_t n)
{
   uint8_t i;
   
   for( i = 0 ; i < n ; i++ )
   {
      SPI_TxByte(data[i]);      
   }
}


/* Reads 'n' bytes from the SPI bus and places them into data
 * data must be of correct lenght. */
void SPI_RxBlock(uint8_t* data, uint8_t n)
{
   uint8_t i;
   
   for( i = 0 ; i < n ; i++ )
   {
      data[i] = SPI_RxByte();      
   }
}



uint8_t SPI_RxByte(void)
{
   return SPI_TxByte(0xFF);
}



