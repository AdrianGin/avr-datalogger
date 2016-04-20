/* ADC1213 Routines for the ATMEGA Micro
 * Author: Adrian Gin
 * Date: 27/06/07
 *
 */

#include <avr/io.h>
#include <util/delay.h>


#include "SPI/spi.h"
#include "ads1213.h"
#include "hardUart/hardUart.h"


int16_t ADS1213_DecimationRatio = 0x1FF;
uint32_t ADS1213_RawReference = 0x00000000;

/* Internal function */
uint8_t ADS1213_TxByte(uint8_t data)
{
	uint8_t inputByte;
	//SPCR |= ((1 << SPR1) );

	/* Set to a slower speed */
	inputByte = SPI_TxByte(data);

	return inputByte;
}

/* Performs a Power on Reset */
//See Figure 27 in Datasheet.
void ADS1213_POR(void)
{
	SPCR &= ~(1 << SPE);

	SPI_DDR |= (1 << SCK);
	//SPI_DDR |= ((1 << MOSI) | ( 1<< MISO) );
	//SPI_PORT &= ~((1 << MISO) | ( 1 << MOSI));
	SPI_PORT &= ~(1 << SCK);
	_delay_ms(30);
	/* Perform a reset */
	SPI_PORT |= (1 << SCK);
	_delay_us(ADS1213_t3);
	SPI_PORT &= ~(1 << SCK);
	_delay_us(ADS1213_t2);
	SPI_PORT |= (1 << SCK);
	_delay_us(ADS1213_t1);
	SPI_PORT &= ~(1 << SCK);
	_delay_us(ADS1213_t2);
	SPI_PORT |= (1 << SCK);
	_delay_us(ADS1213_t3);
	SPI_PORT &= ~(1 << SCK);
	_delay_us(ADS1213_t2);
	SPI_PORT |= (1 << SCK);
	_delay_us(ADS1213_t4);

	SPI_PORT &= ~(1 << SCK);

	SPCR |= (1 << SPE);

}

/* Resets the modulator count to zero */
uint8_t ADS1213_Reset(void)
{
	uint8_t data = (1 << ADS1213_SDL) | (1 << ADS1213_DSYNC) | (ADS1213_REF_OUT_ON) | (ADS1213_TWO_COMP);
	return ADS1213_VerifiedWrite(ADS1213_CMD_REG_3, &data, 1);
}

/* Note SPI_Init() must be called prior */
void ADS1213_Init(void)
{
	/* Set Chip Select as Output */
	ADS1213_CS_DDR |= (1 << ADS1213_CS_PIN);

	const uint8_t data[] = {
			(1 << ADS1213_SDL) | (1 << ADS1213_DSYNC) | (ADS1213_TWO_COMP) | (ADS1213_REF_OUT_ON),
			ADS1213_SELF_CALIB,
			(1 << ADS1213_SF2) | (ADS1213_DecimationRatio >> 8),
			(ADS1213_DecimationRatio & 0xFF)};



	/* CPHA = 1 is required  for this ADC,
	 * CPHA = 1 required for the DS1305 as well (DONE IN SPI INIT)*/

	/* Need to set SDL to SDOUT and turn off REFerence Output */
	/* Perform a Self Calibration calibration */
	/* Always use Turbo 16 as it offers better performance */
	/* Set SF2 to 1 to enable Turbo 16 mode */
	/* Set decimation ratio to 500 which means fData ~=~ 500 Hz */
	/* Have a data rate of 2kHz */
	if( ADS1213_VerifiedWrite(ADS1213_CMD_REG_3, data, 4) == ADS1213_VERIFICATION_ERR )
	{
		uartTxString_P( ("VERIFY ERROR\n") );
	}

}

/* Obtains a 32_bit result, however only the 1st 22 bits will
 * contain data.
 *
 * ADS1213_BUSY will be returned if the read fails */
uint32_t ADS1213_GetResult(void)
{
	uint8_t BusyFlag;
	uint8_t DRDY_Status = 0;


	uint16_t retry;

	static ADS1213Data_t Data;
	Data.result = 0;


	for (retry = 0; retry < 250; retry++)
	{

		ADS1213_ReadInstruction(ADS1213_CMD_REG_3, &DRDY_Status, 1);
		_delay_us(ADS1213_t20_23);
		if (!(DRDY_Status & (1 << ADS1213_DRDY)))
		{
			BusyFlag = 0;
			break;
		}
	}

	if (BusyFlag)
	{
		return ADS1213_BUSY;
	}


	_delay_us(10);
	ADS1213_CS_PORT &= ~(1 << ADS1213_CS_PIN);
	/* Ask to read three bytes from DOR(1) */
	_delay_us(10);
	ADS1213_TxByte((1 << ADS1213_RW) | (1 << ADS1213_MB1));
	_delay_us(10);
	Data.byte[2] = ADS1213_RxByte();
	Data.byte[1] = ADS1213_RxByte();
	Data.byte[0] = ADS1213_RxByte();
	/* Release the chip */
	ADS1213_CS_PORT |= (1 << ADS1213_CS_PIN);

	Data.byte[4] = 0x00;

	/* Truncate data to 22 bits */
	return Data.result & 0xFFFFFF;

}

/* Receives a 24bit integer in the form of an unsigned 32bit integer and
 * translates it into a signed 32bit integer. */
int32_t uint24_tSign(ADS1213Data_t data)
{

	//data.result = (data.result << 8);

#if 1
	// If the 24bit is negative...
	if (data.byte[2] & ADS1213_SIGN_BIT)
	{
		// make it a 32_bit negative
		data.result = (~data.result + 1) & (0x007FFFFF);
		data.result = -data.result;
	}
	return data.result;
#endif

	return (int32_t)(data.result / 256);
}

void ADS1213_CS_Pulse(void)
{
	ADS1213_CS_PORT |= (1 << ADS1213_CS_PIN);
	_delay_us(22);
	ADS1213_CS_PORT &= ~(1 << ADS1213_CS_PIN);
	_delay_us(22);
}



uint8_t ADS1213_VerifiedWrite(uint8_t address, const uint8_t* data, uint8_t nBytes)
{
	uint8_t result = 0x00;
	uint8_t readData[4];
	uint8_t i;

	ADS1213_WriteInstruction(address, data, nBytes);
	ADS1213_ReadInstruction(address, &readData, nBytes);

	for(i = 0; i < nBytes; ++i)
	{
		if( data[i] != readData[i] )
		{
			result = ADS1213_VERIFICATION_ERR;
		}
	}

	return result;
}


void ADS1213_WriteInstruction(uint8_t address, const uint8_t* data, uint8_t nBytes)
{

	if( nBytes < 1 || nBytes > 4 )
	{
		return;
	}

	ADS1213_SELECT();
	//t24 in datasheet
	_delay_us(ADS1213_t24);


	ADS1213_TxByte(ADS1213_WRITE_INS | address | ((nBytes-1) << ADS1213_MB0) );
	//t19 in datasheet
	_delay_us(ADS1213_t19);
	uint8_t i;
	for( i = 0; i < nBytes; ++i)
	{
		ADS1213_TxByte(data[i]);
	}
	/* Release CS */
	_delay_us(ADS1213_t20_23);
	ADS1213_DESELECT();

}



void ADS1213_ReadInstruction(uint8_t address, uint8_t* data, uint8_t nBytes)
{

	if( nBytes < 1 || nBytes > 4 )
	{
		return;
	}

	ADS1213_SELECT();
	//t24 in datasheet
	_delay_us(ADS1213_t24);


	ADS1213_TxByte(ADS1213_READ_INS | address | ((nBytes-1) << ADS1213_MB0) );

	//t19 in datasheet
	_delay_us(ADS1213_t19);

	uint8_t i;
	for( i = 0; i < nBytes; ++i)
	{
		data[i] = ADS1213_RxByte();
	}
	/* Release CS */
	_delay_us(ADS1213_t20_23);
	ADS1213_DESELECT();
}

/* Turns the Vreference and VBias Off as well as puts the unit
 * into sleep mode*/
void ADS1213_Shutdown(void)
{
	const uint8_t data[] = {(1 << ADS1213_SDL), ADS1213_SLEEP };
	ADS1213_WriteInstruction(ADS1213_CMD_REG_3, (const uint8_t*)(&data), 2);
}

void ADS1213_Startup(void)
{
	const uint8_t data[] = {(1 << ADS1213_SDL) | (1 << ADS1213_DSYNC) | (ADS1213_TWO_COMP) | (ADS1213_REF_OUT_ON), ADS1213_SELF_CALIB };
	/* Need to set SDL to SDOUT and turn on REFerence Ouput */
	/* Do a Self Calib */
	ADS1213_WriteInstruction(ADS1213_CMD_REG_3, (const uint8_t*)(&data), 2);
}

void ADS1213_PsuedoCalib(void)
{
	const uint8_t data = ADS1213_PSEUDO_CALIB;
	ADS1213_WriteInstruction(ADS1213_CMD_REG_2, &data, 1);
}


void ADS1213_OpMode(const uint8_t data)
{
	uint8_t cmdByte;
	ADS1213_ReadInstruction(ADS1213_CMD_REG_2, &cmdByte, 1);

	cmdByte &= ~(ADS1213_OPMASK);
	cmdByte |= (data & ADS1213_OPMASK);

	ADS1213_WriteInstruction(ADS1213_CMD_REG_2, &cmdByte, 1);
}


void ADS1213_SetChannel(uint8_t ch)
{
	uint8_t cmdByte;
	ADS1213_ReadInstruction(ADS1213_CMD_REG_2, &cmdByte, 1);
	cmdByte &= ~(ADS1213_CHMASK);
	cmdByte |= (ch & ADS1213_CHMASK);
	ADS1213_WriteInstruction(ADS1213_CMD_REG_2, &cmdByte, 1);
}














