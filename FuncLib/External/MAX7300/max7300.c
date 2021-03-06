
#include <Util/delay.h>
#include "max7300.h"
#include "UI/UI.h"
#include "I2C/i2c.h"



/* Returns the state of the data in 'reg'*/
uint8_t MAX7300_ReadRegister(uint8_t address, uint8_t reg)
{
   uint8_t readByte;


	i2cAddress(address, TW_WRITE);     
 	i2cTransmit(reg);

	
	i2cAddress(address, TW_READ);
   readByte = i2cRead(ACK_BIT);    
 	return readByte;   
}

/* Places 'data' into register 'reg' */
void MAX7300_SetRegister(uint8_t address, uint8_t reg, uint8_t data)
{
	i2cAddress(address, TW_WRITE);     
 	i2cTransmit(reg);
 	i2cTransmit(data);
	i2cStop();
}





