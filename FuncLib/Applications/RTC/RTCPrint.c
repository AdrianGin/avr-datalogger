/* Functions require the use of the UART */
/* Used to write time data to the UART */

#include <avr/pgmspace.h>

#include "I2C/i2c.h"
#include "hardUart/hardUart.h"
#include "RTCPrint.h"
#include "mmculib/uint8toa.h"

#define ROM_LOC PROGMEM

static const char ROM_LOC Monday[] = "Monday";
static const char ROM_LOC Tuesday[] = "Tuesday";
static const char ROM_LOC Wednesday[] = "Wednesday";
static const char ROM_LOC Thursday[] = "Thursday";
static const char ROM_LOC Friday[] = "Friday";
static const char ROM_LOC Saturday[] = "Saturday";
static const char ROM_LOC Sunday[] = "Sunday";

static const PGM_P WEEKDAYS[7] = {  Sunday,
												Monday,
									         Tuesday,
									         Wednesday,
									         Thursday,
									         Friday,
									         Saturday,
									         };
									 
									 
							 


/* printTIMEDATE: */
/* Print out variables with a separator 
 * Each variable is converted to ASCII before being printed
 *
 */
void printTIMEDATE(uint8_t timeDATA, uint8_t separator)
{
   uint8_t	OutputString[10];
	uint8toa(timeDATA, OutputString);
	if(timeDATA < 10)
	{
		uartTx('0');
	}

	uartTxString(OutputString);
	uartTx(separator);

}


/** Receives the time array and converts the HH:MM:SS section
 * into decimal. The result is placed in the convTime address */
void RTC_ConvertTime(uint8_t* time, uint8_t* convTime)
{
   uint8_t i;
   
   /* Ignore the 12/24 hour bit */
	time[HOURS] &= ~(HOURS24);   
      
	for( i = SECONDS; i <= HOURS; i++)
	{
		convTime[i] = ((time[i] & TENS_MASK) >> 3)*5;
		convTime[i] = convTime[i] + (time[i] & ONES_MASK);
	}
}

/** Receives the time array and converts the "DAY" DD/MM/YY section
 * into decimal. The result is placed in the convTime address */
void RTC_ConvertDate(uint8_t* time, uint8_t* convDate)
{
   
   uint8_t i;
   
	for( i = DAY; i <= YEAR; i++)
	{
		convDate[i-DAY] = ((time[i] & TENS_MASK) >> 3)*5;
		convDate[i-DAY] = convDate[i - DAY] + (time[i] & ONES_MASK);
	}    
}



/* printTime:
 * Print out the time in the format HH:MM:SS
 * when passed the time array pointer
 */
void printTime(uint8_t* time)
{

	uint8_t temp_time[3];
	uint8_t i;
	
	/*Convert all times from BCD to decimal */
	RTC_ConvertTime( time , temp_time);
	/* Print out the time in the format HH:MM:SS */
	uartTxString_P( PSTR("\rThe current time is: "));
	for( i = HOURS; i <= HOURS; i--)
	{		
		printTIMEDATE(temp_time[i], ':');
	}
	
}


/* printDate:
 * Print out the Day and Date in the format <DAY OF WEEK> DD:MM:YY
 * when passed the time array pointer
 */
void printDate(uint8_t* time)
{
	
	uint8_t temp_time[4];
	uint8_t i;
	
	/*Convert all dates from BCD to decimal */
	RTC_ConvertDate( time, temp_time);
	/* Print out the day of the week*/
	uartTxString_P( PSTR("\rThe current date is: ") );
	
	uartTxString_P( WEEKDAYS[temp_time[DAY-DAY]-1] );

	uartTx(' ');
	
	/* Print out the Date in the format DD/MM/YY*/
	for( i = DATE; i <= YEAR; i++)
	{		
		printTIMEDATE(temp_time[i-DAY], '/');
	}

}
