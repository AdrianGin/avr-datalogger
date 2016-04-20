/* PWM Routines for the AVRMega 8 */
/* See pwm.h for a description of operation */


#include <avr/io.h>
#include <stdint.h>
#include "pause.h"
#include "pwm.h"

void pwmTimer1AEnable(uint16_t pwmDutyCycle)
{
	 
	 DDRB    |= ( 1<<(PB1) );
    TCCR1A  |= (1<< (COM1A1) );
    TCCR1A  &= ~(1<< (COM1A0) );

 	 OCR1A = pwmDutyCycle;

}


/* Set the OC1A Pin as a PWM output, and set Duty Cycle */
//void pwmTimer1AEnable(uint16 pwmDutyCycle, uint8 pwmOutputPin)
//{
//	 uint8 i;
//    i = 2*pwmOutputPin;	 
//
//	 DDRB    |= ( 1<<(PB1 + pwmOutputPin) );
//    TCCR1A  |= (1<< (COM1A1 - (i) ) );
//    TCCR1A  &= ~(1<< (COM1A0 - (i)) );
	
//    if(pwmOutputPin == 0)
//    {OCR1A = pwmDutyCycle;}
//
//	 if(pwmOutputPin == 1)
//	 {OCR1B = pwmDutyCycle;}
//}

/* Set the OC1B Pin as a PWM output, and set Duty Cycle */
void pwmTimer1BEnable(uint16_t pwmDutyCycle)
{
    DDRB    |= (1<<PB2);
    TCCR1A  |= ( (1<<COM1B1) );
    TCCR1A  &= ~(1<<COM1B0);
    OCR1B   = pwmDutyCycle;
}




/* Disable the OC1A PWM output */
void pwmTimer1ADisable(void)
{
    TCCR1A  &= ~( (1<<COM1A1) | (1<<COM1A0) );
}
/* Disable the OC1B PWM output */
void pwmTimer1BDisable(void)
{
    TCCR1A  &= ~( (1<<COM1B1) | (1<<COM1B0) );
}

/* Set Timer1's prescaler, set PWM mode to Fast PWM and set PWM frequency */
void pwmTimer1Initialise(uint16_t pwmTopCount, uint8_t pwmPreScaler)
{
    /*Set to Fast PWM with ICR being the Top.*/
    TCCR1A |= ( (1<<WGM11) ); 
    TCCR1A &= ~( (1<<WGM10) );
    TCCR1B |= (  (1<<WGM12) | (1<<WGM13) );

    /* Set the Timer1 Prescaler */
    switch (pwmPreScaler)
    {
    case NoPreScale:
    TCCR1B |= (1<<CS10);
    TCCR1B &= ~( (1<<CS11) | (1<<CS12) );
    break;
    
    
    case PreScale8:
    TCCR1B |= (1<<CS11);
    TCCR1B &= ~( (1<<CS10) | (1<<CS12) );
    break;
    
    case PreScale64:
    TCCR1B |= (1<<CS10 | (1<<CS11));
    TCCR1B &= ~( (1<<CS12) );
    break;
    
    case PreScale256:
    TCCR1B |= (1<<CS12) ;
    TCCR1B &= ~( (1<<CS11) | (1<<CS10) );
    break;    
    
    
    case PreScale1024:
    TCCR1B |= ( (1<<CS10) | (1<<CS12) );
    TCCR1B &= ~( (1<<CS11) );
    break;

    }
    
    ICR1 = pwmTopCount;
}

