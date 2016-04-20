
#include <stdint.h>
#include <string.h>
#include <avr/pgmspace.h>
#include "floatFunctions.h"
#include "uint16toa.h"


/** Multiplies the passed float by 2^(adjust) 
  * The 8 exponent bits are held in the 
  * 1st 7 bits of the 4the byte and the last bit of the 3rd byte.
  */
float32_t floatExponent(float32_t data, int8_t adjust)
{
   uint8_t newExponent;
   
   newExponent = ((data.byteField[3] & FLOAT_EXP_BITS_HI) << 1)
                  | ((data.byteField[2] & FLOAT_EXP_BITS_LO) >> 7);
                   
   newExponent += adjust;
   /* Clear the Exponent Bits */
   data.byteField[3] =  (data.byteField[3] & ~(FLOAT_EXP_BITS_HI)) | (newExponent >> 1);
   data.byteField[2] =  (data.byteField[2] & ~(FLOAT_EXP_BITS_LO))  | (newExponent << 7);   
   
   return data;  
}


/** 16bit integer and decimal is the largest we can print */
void printFloat(float data, uint8_t* string)
{
   float32_t working; 
	working.FP = data;
	if( working.byteField[3] & FLOAT_SIGN_BIT)
	{
   	string[0] = '-';
		data = -data;
		working.byteField[3] &=  (~FLOAT_SIGN_BIT);
	}
	else
	{
		string[0] = '+';	
	}

   uint16toa( working.FP, (char*)&string[1], 0); 
   strcat_P((char*)string, PSTR(".") );
   
	working.FP = data - (int32_t)data;
	float_4dp( working.FP * MUTLIPLIER, (char*)&string[strlen((char*)string)]); 
}


/** Returns a float from address short. */
float xpgm_read_float(uint16_t* address_short)
{
   float32_t* floatPtr = (float32_t*)(address_short);;   
   float32_t returnFloat;
   returnFloat.byteField[3] = pgm_read_byte( &floatPtr->byteField[3] );
   returnFloat.byteField[2] = pgm_read_byte( &floatPtr->byteField[2]);
   returnFloat.byteField[1] = pgm_read_byte( &floatPtr->byteField[1]);
   returnFloat.byteField[0] = pgm_read_byte( &floatPtr->byteField[0]);         
   return returnFloat.FP;
}






/** 16bit integer and decimal is the largest we can print */
void print3impliedDP(float data, uint8_t* string)
{
   float32_t working;
	working.FP = data;
	if( working.byteField[3] & FLOAT_SIGN_BIT)
	{
   	string[0] = '-';
		data = -data;
		working.byteField[3] &=  (~FLOAT_SIGN_BIT);
	}
	else
	{
		string[0] = '+';
	}

   uint16toa( working.FP, (char*)&string[1], 0);
   strcat_P((char*)string, PSTR(".") );

	working.FP = data - (int32_t)data;
	float_4dp( working.FP * MUTLIPLIER, (char*)&string[strlen((char*)string)]);
}
