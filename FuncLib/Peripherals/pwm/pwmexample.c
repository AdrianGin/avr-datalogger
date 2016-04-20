#include <avr/io.h>
#include <stdint.h>
#include "pause.h"
#include "pwm.h"

/* Fade LEDs on the OC1A and B pins in and out */

int main(void)
{
    DDRD = 0xFF;
    
    pwmTimer1Initialise(128,TIMER1_CLK_DIV1);
    pwmTimer2Initialise(TIMER2_CLK_DIV8);

		pwmTimer1BEnable(128);
		pwmTimer1AEnable(128);

    uint16_t i;
    while(1)
    {
    	for (i = 0; i < 255 ; i++)
		{
        pwmTimer2Enable(i);

		  pwmTimer1BEnable( i>128 ? 255-i : i);

		  if( (i > 64) )
		  {
			pwmTimer1AEnable( (2*(128-i) ));
		  }
		  else
		  {
			pwmTimer1AEnable(2*i); 
		  }

		  if( (i > 127) )
		  {
			pwmTimer1AEnable((2*(i-127)));
		  }

		  if( (i > 192) )
		  {
			pwmTimer1AEnable( (2*(255-i)) );
		  }


	     pausems(20);
		

    	}

    	for (i = 255; i > 0; i--)
		{
        pwmTimer2Enable(i);
		  
		  pwmTimer1BEnable( i>128 ? 255-i : i);

		  if( (i > 64) )
		  {
			pwmTimer1AEnable( (2*(128-i) ));
		  }
		  else
		  {
			pwmTimer1AEnable(2*i); 
		  }

		  if( (i > 127) )
		  {
			pwmTimer1AEnable((2*(i-127)));
		  }

		  if( (i > 192) )
		  {
			pwmTimer1AEnable( (2*(255-i)) );
		  }


	     pausems(20);

    	}

    }
    return 0;


}

