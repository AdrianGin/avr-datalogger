#ifndef	_RTCPRINT_ROUTINES
#define	_RTCPRINT_ROUTINES


#include "Time\gettime.h"

#define	SECONDS 	(0)
#define	MINUTES 	(1)
#define	HOURS		(2)
#define	DAY		(3)
#define	DATE		(4)
#define	MONTH		(5)
#define	YEAR		(6)

#define	HOURS24	(0x40)

/* BCD to Decimal macro */
/* These are used to convert the tm structure values to 
 * the format accepted by the Dallas RTCs */
#define	DEC2BCD_HI(x)	((x)/10)
#define	DEC2BCD_LO(x)	((x)%10)
#define	DEC2BCD(x)		(((DEC2BCD_HI(x)) << 4) + (DEC2BCD_LO(x)))

#define	BCD2DEC(x)		((x >> 4)*10) + ((x & 0x0F))

#define	BCD_SECONDS		DEC2BCD(TM_SECONDS)
#define	BCD_MINUTES		DEC2BCD(TM_MINUTES)
#define	BCD_HOURS		DEC2BCD(TM_HOURS)
#define	BCD_DAY			DEC2BCD(TM_DAY)
#define	BCD_DATE			DEC2BCD(TM_DATE)
#define	BCD_MONTH		DEC2BCD(TM_MONTH)
#define	BCD_YEAR			DEC2BCD(TM_YEAR)

#define	TENS_MASK	(0x70)
#define	ONES_MASK	(0x0F)
#define	HOUR_MASK	(0x30)


typedef struct _time
{
   uint8_t seconds;
   uint8_t minutes;
   uint8_t hours;
   uint8_t dayOfWeek;
   uint8_t date;
   uint8_t month;
   uint8_t year;  
} Time_t;


/* printTime:
 * Outputs to the UART the time in a pre-defined format*/
void printTime(uint8_t* time); 

/* printDate:
 * Outputs to the UART the day and date in a pre-define format*/
void printDate(uint8_t* time);

/* printTIMEDATE: */
/* Print out variables with a separator 
 * Each variable is converted to ASCII before being printed
 * Internal function 
 */
void printTIMEDATE(uint8_t timeDATA, uint8_t separator);

void RTC_ConvertDate(uint8_t* time, uint8_t* convDate);

void RTC_ConvertTime(uint8_t* time, uint8_t* convTime);


#endif
