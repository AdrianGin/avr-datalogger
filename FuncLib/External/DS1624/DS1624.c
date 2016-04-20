/* Filename: 				DS1624.c
 * Author:   				Adrian Gin (amg94@student.canterbury.ac.nz)
 * Created/Modified: 		26-06-06
 *
 * Description:
 *		Functions used to interface with the DS1624 Temperature Sensor IC.
 *		These functions utilise the hardware TWIregisters
 * 	I2C / TWI serial routines for the AVR mpu's.
 * 	For AVR Core
 */
 
 
#include <avr/io.h>
#include <stdint.h>
#include <stdlib.h>

#include "../I2C/i2c.h"
#include "../DS1624/DS1624.h"
#include "../hardUart/hardUart.h"


static uint8_t	OutputString[10];

	

/* getTemp:
 * Obtain the temperature from the DS1624 IC
 * and return the pointer to where it is stored
 */
int8_t* getTemp(void)
{
	static int8_t	temperature[2];
	
	i2cAddress(DS1624_ADDRESS, TW_WRITE);
	/* Intiate Read Temperature */
	i2cTransmit(0xAA); 	
		
	/* Receive the bytes*/
	i2cAddress(DS1624_ADDRESS, TW_READ);
	i2cReadBlock(temperature, 2);
	i2cStop();
	
	
	return temperature;
}

/* printlower8bits:
 * Prints the lower eight bits of the temperature sensor
 * The passed parameter is to be the lower 8bits obtained from the DS1624.
 */
void printlower8Bits(uint8_t temperature)
{
		
	 	temperature = temperature >> 3;
	 	utoa(temperature, OutputString, 10);
	 	uartTxString("\n\rThe Raw value of the lower 8 bits are: ");
	 	uartTxString(OutputString);
	 	uartTx('\n');
	 	uartTx('\r');	
}





/* printTemperature:
 * Prints the temperature via the hardware UART.
 */
void printTemperature(uint8_t* temperature)
{
		/*Convert Temperature to signed temperatures*/	 	
	 	sign_temperature(temperature);
		
		/* Print the first 3 significant digits */ 	
		itoa(temperature[0], OutputString, 10);
		OutputString[4] = 0;
		uartTxString("\n\rThe Temperature is: ");
		uartTxString(OutputString);
		
		/*Print Decimal Point*/
		uartTx('.');
		
		/*Print the last two significant digits*/
		utoa(temperature[1], OutputString, 10);
		if(temperature[1] < 10)
		{
			uartTxString("0");
		}
		
		uartTxString(OutputString);		
		uartTxString(" Degrees Celcius!! \n\r");	
	
}



/* signed_temperature:
 * Calculate the temperature returned from a DS1624 Temperature sensor
 * See the DS1624 Datasheet for more information.
 */
void sign_temperature(uint8_t* unsigned_temperature)
{
	/* Perform two's complement on the integers if they are 'negative'*/
	if(unsigned_temperature[0] > 127)
	{
		unsigned_temperature[0] = -((~unsigned_temperature[0]) + 1);
		unsigned_temperature[1] = ((~unsigned_temperature[1]) + 1);		
	}	
	
	unsigned_temperature[1] = unsigned_temperature[1] >> 3;
	unsigned_temperature[1] = unsigned_temperature[1] * 3;
	
}


