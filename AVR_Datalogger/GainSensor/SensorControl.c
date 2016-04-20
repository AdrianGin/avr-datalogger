#include <avr/io.h>
#include <avr/pgmspace.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "SensorControl.h"
#include "GainControl.h"
#include "hardUart/hardUart.h"
#include "mmculib/floatFunctions.h"
#include "ADS1213/ads1213.h"

#define RESULT_DEBUG 1

/* Sensor Area */

/* Require a function which 'auto' scales the input data */
/* Also needs to have an array of either Voltage, Temperature or Off channels */

/* Sensor state is ON/OFF */
/* Sensor Type is VOLTAGE / TEMPERATURE */
uint32_t SensorType;
uint32_t SensorState;

/* The sensor gain selects which index of the SENSOR_GAIN array is used */
/* Each sensor will use 4_bits thus there are 16 combo's of gains */
static uint8_t SensorGain[SENSOR_COUNT/2];


/* Sensor Function channels are ranged between 0 -> 31 inclusive */

void SensorOn(uint8_t channel)
{
   SensorState |= ( (uint32_t)(1) << channel);
}

void SensorOff(uint8_t channel)
{
   SensorState &= ~((uint32_t)(1) <<channel);
}

void SensorToggle(uint8_t channel)
{
	SensorState = SensorState ^ ((uint32_t)(1) << channel);	
}

void SensorSetType(uint8_t channel, uint8_t type)
{
	SensorType &= ~( (uint32_t)(1) << channel);
	SensorType |= ( (uint32_t)(type) << channel);	
}

void SensorTypeToggle(uint8_t channel)
{
	SensorType = SensorType ^ ((uint32_t)(1) << channel);		
}

uint8_t SensorGetType(uint8_t channel)
{
   return (SensorType & ((uint32_t)(1) << channel)) ? SENSOR_VOLTAGE : SENSOR_TEMP ;
}

uint8_t SensorGetState(uint8_t channel)
{
   return (SensorState & ((uint32_t)(1) << channel)) ? SENSOR_ON : SENSOR_OFF;
}


/* Returns the index of SENSOR_GAIN */
uint8_t SensorGetGain(uint8_t channel) 
{
	uint8_t returnVal;
	returnVal = SensorGain[channel / 2] >> (4*(channel % 2));

   return (returnVal & 0x0F);
}


/* Each sensor is assigned a gain, this sets which of the
 * SENSOR_GAIN's are used. gain must be within the SENSOR_GAIN index */
void SensorSetGain(uint8_t channel, uint8_t gain)
{
   uint8_t nibble;
   
   nibble = 4* (channel % 2);
   
	/* To use the correct nibble */
	gain = gain & 0x0F;
	SensorGain[ (channel / 2) ] &= ~( 0x0F << nibble);
	SensorGain[ (channel / 2) ] |= (gain << nibble);
	
}

#if 0
/* Ensures that the ADC result is as large as possible without being saturated */
void SensorAutoScale(uint8_t channel)
{
   uint8_t i;
   uint8_t gain;
   int32_t ADCResult;
   
   for(i = 0; (gain = pgm_read_byte( &GS_GAIN[i] )); i++)
   {
		/* Start from the highest gain and work down */
      SensorSetGain(channel, gain );
      ADCResult = (int32_t)SensorData(channel);
      
      /* Within the saturation limit of 15 bits (could use 16?) */
      if(abs(ADCResult) > SENSOR_SATURATION_LIMIT)
      {
			if( i != 0 )
			{
				SensorSetGain(channel, pgm_read_byte(GS_GAIN[i-1]) );
			}
			
         return;
      }
   }
}
#endif

/* Turns all digits into voltages regardless of Type */
/* Function assumes a signed 24bit integer */
float32_t SensorCondition(uint32_t data, uint8_t gainIndex)
{

	/* Could change to 64's */
	char outputString[20];
	
	ADS1213Data_t ConvertedData;
	int32_t signedData;
	float32_t   dataFP;
	float32_t   gainFP;
	
	/* Convert signed 24bit into signed 32bit */
	ConvertedData.result = data;
	signedData = uint24_tSign( ConvertedData );
	
	//signedData -= GAIN_OFFSETS[gainIndex];
		
/** Debug only because the float output function can only handle 16bit */		   
	dataFP.FP = (float)(signedData);

#if RESULT_DEBUG
   ltoa( data, outputString, 10);
   uartTxString( (uint8_t*)"Raw Data = ");
   uartTxString(outputString);      
   uartNewLine(); 
   
   ltoa( signedData , outputString, 10);
   uartTxString( (uint8_t*)"Signed Data = ");
   uartTxString(outputString);      
   uartNewLine();

    
   uartTxString( (uint8_t*)"Float Data = ");
   printFloat( dataFP.FP, outputString );
   uartTxString(outputString);    
   uartNewLine();
#endif   
/** Debug only */	
   
   gainFP = (float32_t)pgm_read_float( (uint16_t*) &SENSOR_GAIN[gainIndex] );

#if RESULT_DEBUG   
/** Debug only */	   
   uartTxString( (uint8_t*)"Current Gain = ");
   printFloat( gainFP.FP, outputString );
   uartTxString(outputString);    
   uartNewLine();
#endif   
/** Debug only */	   
   
   /* Divide by 2^22 and multiply by 256 */
   dataFP.FP = dataFP.FP / (SENSOR_REF_MULTIPLIER * gainFP.FP);
	dataFP = floatExponent(dataFP, - (SENSOR_ENOB) + 8);
   
#if RESULT_DEBUG  
   uartTxString( (uint8_t*)"Conditioned 'Voltage' is: "); 
   printFloat(dataFP.FP, outputString);
   uartTxString(outputString);   
   uartNewLine(); 
#endif      							
   
   return dataFP; 
    
}





















