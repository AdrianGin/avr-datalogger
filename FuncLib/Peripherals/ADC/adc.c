/* ADC.c
 * ADC Routines for the AVR Mega8
 *
 * Date: 16/04/06
 * Author : Adrian Gin (amg94@student.canterbury.ac.nz)
 *
 *
 */

/* User is responsible for the ADC Input pin being set as an input*/ 
 
/* To perform conversion either call: 
 * adcInitialise();
 * adcSetPin(ADC Input);
 * result = adc10(); (or adc8();) 
 *
 */

/* To perform conversion either call: 
 * adcSetPrescale(prescale);
 * adcSetReference(reference);
 * adcSetPin(ADC Input)
 * adcOn();
 * result = adc10(); (or adc8();) 
 *
 */

#include <stdint.h>
#include <avr/io.h>
#include "adc.h"


/* adcInitialise
 *
 * Sets the default ADC prescale and reference */
void adcInitialise(void)
{
	adcSetPrescale(ADC_PRESCALE_DEFAULT);
	adcSetReference(ADC_REF_DEFAULT);
	adcOn();
}

/* adcSetPrescale
 *
 * Sets the ADC clock to divide the F_CLK.
 */
void adcSetPrescale(uint8_t prescale)
{
	ADCSRA = ((ADCSRA & (~ADC_PRESCALE_MASK)) | prescale);
}

/* adcSetReference
 *
 * Reference voltage can either be, AREF, VCC or 2.56V Internal reference
 */
void adcSetReference(uint8_t reference)
{
	ADMUX = ((ADMUX & (~ADC_REF_MASK)) | (reference << REFS0));
}

/* adcSetPin
 *
 * Sets the adc pin on which the conversion takes place.
 */
void adcSetPin(uint8_t ADCPin)
{
	ADMUX = ((ADMUX & (~ADC_PIN_MASK)) | (ADCPin));
}

/* adcStartConversion
 *
 * Performs one ADC conversion
 */
/* One shot ADC mode conversion */
void adcStartConversion(void)
{
	ADCSRA = (ADCSRA & (~(1<<ADFR))) | (1<<ADSC);
	while(ADCSRA & (1<<ADSC)){;}
}

/* adcOff
 *
 * Turns off the ADC module to save power
 */
void adcOff(void)
{
	ADCSRA &= ~(1<<ADEN);
}

/* adcOn
 *
 * Turns on the ADC module
 */
void adcOn(void)
{
	ADCSRA |= (1<<ADEN);
}

/* adc10
 *
 * Returns a 10bit ADC value
 */
uint16_t adc10(void)
{
	/* Set the result to right adjust (LSB at ADC:0) */
	ADMUX &= ~(1<<ADLAR);
	adcStartConversion();	
	return ADC;
}

/* adc8
 *
 * Returns a 8bit ADC value
 */
uint8_t adc8(void)
{
	/* Set the result to left adjust (LSB at ADC:2) */
	ADMUX |= (1<<ADLAR);
	adcStartConversion();	
	return ADCH;
}


