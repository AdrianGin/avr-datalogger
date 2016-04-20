/* For delays larger than 'x' msecs, use the RTC Timer 1 */
/* The other RTC Timer 0 will be used for alarms (multiple timers can be used 
 * through software */
 
/* When the RTC is being used to sample, Timer2 Compare interrupt is disabled */
/* SC_DisableTimer is called */

/* Have all interrupts in the main function */

/* Sets up the required timers etc */


#include <avr/io.h>
#include <stdlib.h>
#include <util/delay.h>
#include <avr/pgmspace.h>

#include "ADS1213/ads1213.h"
#include "SampleCtrl.h"
#include "GainSensor/GainControl.h"
#include "GainSensor/SensorControl.h"
#include "mmculib/floatFunctions.h"
#include "hardUart/hardUart.h"
#include "RTC/RTCPrint.h"
#include "DS1305/ds1305.h"
#include "MemMan/memman.h"
#include "DS1305/ds1305.h"
#include "main.h"

#define SENSOR_DEBUG 0

/* Disable Sampling Timer */
SoftTimer_8	SC_MasterTimer = {0,0,0};
SoftTimer_16 SC_INTLongDelay;
uint32_t SC_SampleCount = 0;
static uint8_t SampleMode;


void SC_Init(void)
{
   /* The primary sample timer will use the 8-bit Timer 2 */
   /* Use CTC Mode and clock at FCPU / 32  */
   TCCR2 = (1 << WGM21) | (1<< CS21) | (1<< CS20);    
   /* Set Compare mode to 250 so a compare occurs every 250us */
   OCR2 = SC_COMPARE_RATE;
   TCNT2 = 0x00;
}




/** Samples the Activated Channels with their respective gains 
 * and then writes them to memory */
void SC_Sample(void)
{
   uint8_t i;
   uint8_t chGain;
   static uint32_t ADCValue;
   float32_t sample;
   uint8_t outputString[10];

	/** Write Timestamp */
	DS1305_GetTime(DS1305_TimeDate_config);
	MM_Write( DS1305_TimeDate_config[HOURS] );
	MM_Write( DS1305_TimeDate_config[MINUTES] );		
	MM_Write( DS1305_TimeDate_config[SECONDS] );

		
	

   for( i = 0; i < SENSOR_COUNT; i++)
   {
      if( SensorGetState(i) )
      {        
         /* Set the channel */
         GS_Channel( i + 1 );

         /** Get the corresponding CH Gain and set it */
         chGain = SensorGetGain(i);
         GS_GainSel( pgm_read_byte( &GS_GAIN[chGain]) );

		 if( ADS1213_Reset() == ADS1213_VERIFICATION_ERR )
		 {
			 uartTxString_P( PSTR("VERIFY ERROR\n") );
		 }
         /** Condition the data from the ADC */
         ADCValue = ADS1213_GetResult();
	
         sample = SensorCondition(ADCValue, chGain); 
            /* Debugging */
#if SENSOR_DEBUG            
         uint8toa(i , outputString);
			uartTxString_P( PSTR("Channel: ") );
			uartTxString(outputString);
			uartNewLine();       
         printFloat(sample.FP, outputString);
			uartTxString(outputString);            
         uartNewLine();
#endif		
			if( SensorGetType(i) == SENSOR_TEMP )
			{
				sample.FP = (sample.FP * SC_K_SEEBECK_COEFF_INV);
				/** Ambient Temperature */
				sample.FP = sample.FP + ambientTemperature + SC_K_TEMP_OFFSET;
			}
 				
		
			//MasterDataRecord.sampleCount++;				
			/** Write data to buffer */
			MM_Write(sample.byteField[3]);
			MM_Write(sample.byteField[2]);
			MM_Write(sample.byteField[1]);
			MM_Write(sample.byteField[0]);			
      
       /* End of Debugging */
      }  
   }   
}

uint8_t SC_GetShortRate(void)
{
	return SC_MasterTimer.timeCompare;	
}

uint16_t SC_GetLongRate(void)
{
	return SC_INTLongDelay.timeCompare;
}

/** Valid up to 2 seconds */
void SC_SetSamplingRate(uint8_t tensMilliseconds)
{
	SC_MasterTimer.timeCompare = tensMilliseconds;
	SampleMode = SC_SAMPLE_INTERNAL;
}

/* This function uses the RTC to provide sampling interrupts */
/** The interrupt will clock each second, and the 1 second interrupt
 * is only activated if long delays are used */
void SC_SetSamplingRate_Long(uint16_t seconds)
{
	//SC_MasterTimer.timerEnable = TIMER_DISABLE;
	SampleMode = SC_SAMPLE_EXTERNAL;
	SC_INTLongDelay.timeCompare = seconds;
}


uint8_t SC_GetMode(void)
{
	return SampleMode;
}
