/* ADC1213 Routines for the ATMEGA Micro
 * Author: Adrian Gin
 * Date: 27/06/07
 *
 */

/* File Name	: 'ads1213.h'
 	Title			: Burr Brown 22-bit ADC
 	Author		: Adrian Gin


*/

/** \file ads1213.h
	 \brief Burr Brown 22-bit ADC Interface.
*/

/**	 

	\ingroup avr_external
 	\defgroup ads1213 Burr Brown 22-bit ADC Interface.
 	\code #include "ads1213.h" \endcode

 	 
	 \par Description
	This library allows the use of an SPI controlled ADS1213.
	The ADS1213 is a 22-bit ADC.
 	 
*/
//@{


#ifndef	_ADS1213_ROUTINES
#define	_ADS1213_ROUTINES

#include <stdint.h> 
#include <math.h> 
 
#include "hardwareSpecific.h"

/* Instruction Register Bits */
#define ADS1213_RW         (7)
#define ADS1213_MB1        (6)
#define ADS1213_MB0        (5)
#define ADS1213_A3         (3)
#define ADS1213_A2         (2)
#define ADS1213_A1         (1)
#define ADS1213_A0         (0)

//ADS1213 Register Addresses
#define ADS1213_DATAOUTPUT_REG_2 		(0x00 << ADS1213_A0)
#define ADS1213_DATAOUTPUT_REG_1 		(0x01 << ADS1213_A0)
#define ADS1213_DATAOUTPUT_REG_0 		(0x02 << ADS1213_A0)
#define ADS1213_CMD_REG_3 				(0x04 << ADS1213_A0)
#define ADS1213_CMD_REG_2 				(0x05 << ADS1213_A0)
#define ADS1213_CMD_REG_1 				(0x06 << ADS1213_A0)
#define ADS1213_CMD_REG_0 				(0x07 << ADS1213_A0)
#define ADS1213_OFFSET_CAL_REG_2 		(0x08 << ADS1213_A0)
#define ADS1213_OFFSET_CAL_REG_1 		(0x09 << ADS1213_A0)
#define ADS1213_OFFSET_CAL_REG_0 		(0x0A << ADS1213_A0)
#define ADS1213_FULLSCALE_CAL_REG_2 	(0x0C << ADS1213_A0)
#define ADS1213_FULLSCALE_CAL_REG_1 	(0x0D << ADS1213_A0)
#define ADS1213_FULLSCALE_CAL_REG_0 	(0x0E << ADS1213_A0)

#define ADS1213_1BYTE_INS 				(0x00 << ADS1213_MB0)
#define ADS1213_2BYTE_INS 				(0x01 << ADS1213_MB0)
#define ADS1213_3BYTE_INS 				(0x02 << ADS1213_MB0)
#define ADS1213_4BYTE_INS 				(0x03 << ADS1213_MB0)

#define ADS1213_WRITE_INS 				(0 << ADS1213_RW)
#define ADS1213_READ_INS 				(1 << ADS1213_RW)



#define ADS1213_ENOB		22
#define ADS1213_MAXOUTPUT ( pow(2,ADS1213_ENOB) - 1)



/* Command Register Bits */
/* Byte 3 */
#define ADS1213_BIAS       (7)
#define ADS1213_REFO       (6)
#define ADS1213_DF         (5)
#define ADS1213_UNIPOLAR   (4)
#define ADS1213_SDL        (1)
#define ADS1213_DRDY       (0)
#define ADS1213_DSYNC      (0)

/* Byte 2 */

#define ADS1213_MD2        (7)
#define ADS1213_MD1        (6)
#define ADS1213_MD0        (5)

#define ADS1213_G2		   (4)
#define ADS1213_G1		   (3)
#define ADS1213_G0		   (2)

#define ADS1213_CH1		   (1)
#define ADS1213_CH0		   (0)

#define ADS1213_OPMASK    ((1<<ADS1213_MD2) | (1<<ADS1213_MD1) | (1<<ADS1213_MD0))
#define ADS1213_GMASK     ((1<<ADS1213_G2) | (1<<ADS1213_G1) | (1<<ADS1213_G0))
#define ADS1213_CHMASK    ((1<<ADS1213_CH1) | (1<<ADS1213_CH0))

/* Operating Modes */
#define ADS1213_NORMAL     		((0x00) << (ADS1213_MD0))
#define ADS1213_SELF_CALIB  	((0x01) << (ADS1213_MD0))
#define ADS1213_OFFSET_CALIB  	((0x02) << (ADS1213_MD0))
#define ADS1213_FULLSCALE_CALIB ((0x03) << (ADS1213_MD0))
#define ADS1213_PSEUDO_CALIB    ((0x04) << (ADS1213_MD0))
#define ADS1213_BACKGND_CALIB   ((0x05) << (ADS1213_MD0))
#define ADS1213_SLEEP           ((0x06) << (ADS1213_MD0))

/* Data Formats */
#define ADS1213_TWO_COMP          (0x00 << ADS1213_DF)
#define ADS1213_OFFSET_BIN        (0x01 << ADS1213_DF)

/* Reference Output */
#define ADS1213_REF_OUT_HIZ		  (0x00 << ADS1213_REFO)
#define ADS1213_REF_OUT_ON 		  (0x01 << ADS1213_REFO)

/* Maximum Decimation Ratio */
#define ADS1213_MAX_DCR		(8000)
#define ADS1213_MIN_DCR		(20)

/* Byte 1 */
#define ADS1213_SF2        (7)
#define ADS1213_SF1        (6)
#define ADS1213_SF0        (5)

/* The 23th bit holds the busy signal 
 * ie the 6th bit of Byte[2]*/
#define ADS1213_BUSY       (0x1FFFFFFF)


//SPI Timings
#define ADS1213_min_us_time(clks) ((clks+1) / (ADS1213_CLK/1e6))



//Timings for POR, see Figure 27 in Datasheet
#define ADS1213_t1		ADS1213_min_us_time(600)
#define ADS1213_t2		ADS1213_min_us_time(11)
#define ADS1213_t3		ADS1213_min_us_time(1200)
#define ADS1213_t4		ADS1213_min_us_time(2100)


//Time between last Instruction and first data
#define ADS1213_t19		ADS1213_min_us_time(13)
//Minimum CS hold time (t24) start
#define ADS1213_t24		ADS1213_min_us_time(11)
//Minimum CS hold time (t20+t23) end
#define ADS1213_t20_23		ADS1213_min_us_time(10+2)



//Time between last Data and next Instruction
#define ADS1213_t36		ADS1213_min_us_time(41)


#define ADS1213_SIGN_BIT   (0x80)

#define ADS1213_VERIFICATION_ERR (0x01)

typedef union ADS1213_data 
{
   uint8_t  byte[4];
   uint32_t result;
   float    FPresult;  
} ADS1213Data_t;

extern int16_t ADS1213_DecimationRatio;


#define ADS1213_RxByte() ADS1213_TxByte(0xFF)

/* CPHA = 1 is required  for this ADC,
 * CPHA = 1 required for the DS1305 as well*/

uint8_t ADS1213_TxByte(uint8_t data);

void ADS1213_POR(void);
void ADS1213_PORt1(void);
void ADS1213_PORt2(void);
void ADS1213_PORt3(void);


void ADS1213_Init(void);
uint32_t ADS1213_GetResult(void);
void ADS1213_CS_Pulse(void);
void ADS1213_Shutdown(void);
void ADS1213_Startup(void);
uint8_t ADS1213_Reset(void);
void ADS1213_PsuedoCalib(void);
void ADS1213_OpMode(const uint8_t data);
void ADS1213_SetChannel(uint8_t ch);
int32_t uint24_tSign(ADS1213Data_t data);

void ADS1213_WriteInstruction(uint8_t address, const uint8_t* data, uint8_t nBytes);
void ADS1213_ReadInstruction(uint8_t address, uint8_t* data, uint8_t nBytes);

uint8_t ADS1213_VerifiedWrite(uint8_t address, const uint8_t* data, uint8_t nBytes);

inline void ADS1213_SELECT(void)
{
	ADS1213_CS_PORT &= ~(1 << ADS1213_CS_PIN);
}

inline void ADS1213_DESELECT(void)
{
	ADS1213_CS_PORT |= (1 << ADS1213_CS_PIN);
}


#endif
