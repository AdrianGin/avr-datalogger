

#ifndef	_FLOAT_FUNCTIONS
#define  _FLOAT_FUNCTIONS

#include <stdint.h>

#define MUTLIPLIER	(10000)
#define FLOAT_EXPONENT_BITS   (0x7F8)
#define FLOAT_EXP_BITS_HI     (0x7F)
#define FLOAT_EXP_BITS_LO     (0x80)
#define FLOAT_SIGN_BIT        (0x80)


typedef union _GS_float
{
   uint8_t  byteField[4];
   float    FP;  
   
} float32_t;



/** Allows access to the exponent bits of a floating point
*  number
*  \param adjust Number of bits to shift the exponent bits by
*/
float32_t floatExponent(float32_t data, int8_t adjust);

/** 16bit integer and decimal is the largest we can print */
void printFloat(float data, uint8_t* string);

float xpgm_read_float(uint16_t* address_short);




#endif

