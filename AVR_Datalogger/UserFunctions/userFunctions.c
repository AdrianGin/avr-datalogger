#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <Util/delay.h>
#include <stdlib.h>
#include <string.h>

#include "main.h"

#include "SPI/spi.h"
#include "hardUart/hardUart.h"
#include "SampleCtrl/SampleCtrl.h"
#include "GainSensor/GainControl.h"
#include "GainSensor/SensorControl.h"
#include "ADS1213/ads1213.h"
#include "DS1305/ds1305.h"
#include "RTC/RTCPrint.h"
#include "SD_MMC/sd.h"
#include "uartTerm/uartInput.h"
#include "Menu/Menu.h"
#include "UI_KP/UI_KP.h"
#include "UI_LCD/UI_LCD.h"
#include "MemMan/memman.h"
#include "TinyFS/tff.h"
#include "UserFunctions/userFunctions.h"
#include "Serial_EE/serial_ee.h"
#include "mmculib/uint16toa.h"
#include "mmculib/uint8toa.h"
#include "TMP123/tmp123.h"

static const uint8_t SC_RTC0_CONFIG[] = {0x80, 0x80, 0x80, 0x80};




void Reset(void* data)
{
   asm volatile("jmp 0"::);
}



#if 0
/* Read ADS1213 CMR */
void ADS1213_Status(void* data)
{
   uint8_t ADS1213Byte;
   
   /* Read 4 byte, that is 4 bytes of CMR */
   
   ADS1213_CS_PORT &= ~(1 << ADS1213_CS_PIN);   
   
   ADS1213_TxByte( (1 << ADS1213_RW) 
                   | (1 << ADS1213_A2) 
                   | (1 << ADS1213_MB0)
                   | (1 << ADS1213_MB1) );  
   
   ADS1213Byte = ADS1213_RxByte();
   uartTx(ADS1213Byte);
   ADS1213Byte = ADS1213_RxByte();
   uartTx(ADS1213Byte);
   ADS1213Byte = ADS1213_RxByte();
   uartTx(ADS1213Byte);
   ADS1213Byte = ADS1213_RxByte();
   uartTx(ADS1213Byte);                        
   ADS1213_CS_PORT |= (1 << ADS1213_CS_PIN);      
}
#endif



/* Prints out which channels are gains are selected 
 * and also the relevant voltage / temperature*/
void GS_Status(void* data)
{
	uint8_t passedData[2];
	uint8_t* input;
	
	input = (uint8_t*)data;
		
	passedData[0] = (*input);
	passedData[1] = 0xFF;
	
   ChannelSettings(passedData);
}

void ChannelUp(void* data)
{
   if( channel == 32 )
   {
      channel = 31;  
   }      
   channel = channel + 1;    
   GS_Channel(channel);
   //uartTx(channel);
}


void ChannelDown(void* data)
{
   if( channel == 1 )
   {
      channel = 2;  
   }      
   channel = channel - 1;    
   GS_Channel(channel);
   //uartTx(channel); 
}



uint8_t GainUp(void* data)
{
	gain = gain + 1;
   if( gain >= (GAIN_COUNT-1) )
   {
      gain = GAIN_COUNT - 1;
   }      
   GS_GainSel( pgm_read_byte( &GS_GAIN[gain])  );
   
   return gain;
   //uartTx(gain);
}

uint8_t GainDown(void* data)
{
   if( gain == 0 )
   {
      gain = 1;  
   }      
   gain = gain - 1;
   GS_GainSel( pgm_read_byte( &GS_GAIN[gain])  );
   return gain;  
}


/* Prints out which channels are gains are selected */
void GS_Status2(void* data)
{
   uint8_t outputString[11];
   uint8_t* input = (uint8_t*)data;
   int32_t signedData;
   int32_t gainOffset;
   uint32_t lastestResult;      
   static uint8_t chGain;
   
   
   if( firstEnter == 0 )
   {     
      switch( *input )
      {
         /* Channel Up */
         case 'q':
         case KP_A:
            ChannelUp(0);   
         break;
            
         /* Channel Down */  
         case 'a':                   
         case KP_B:
            ChannelDown(0);
         break;

        /* Gain Up */
        /// case 'w':
         case KP_2:
            chGain = GainUp(0);   
         break;
            
         /* Gain Down */  
         case 's':                   
         case KP_5:
            chGain = GainDown(0);
         break;         
         
         /* Print raw ADC number */
         case 'g':
         case KP_C:

            lastestResult = ADS1213_GetResult();             
          	signedData = uint24_tSign( (ADS1213Data_t) lastestResult );           
            gainOffset = GAIN_OFFSETS[chGain];
         break;
         
         /* Exiting function */
         case KB_BACK:
         case KP_BACK:
          
            MenuSetInput(KP_BACK);
            stateMachine(currentState);
            MenuSetInput(0);
            return;
            
            break;
               
         default:
         break;     
         
      }
   }

   MenuPrint_P( PSTR("Channel: ") );
   uint8toa(channel, outputString);
   MenuPrint(outputString);
   
   MenuNewLine();   

   MenuPrint_P( PSTR("Gain: ") );
   uint8toa(gain, outputString);
   MenuPrint(outputString);   
   MenuNewLine(); 


   ltoa(signedData, outputString, 10);    
   MenuPrint(outputString);         
   MenuNewLine();   

   ltoa(gainOffset, outputString, 10);    
   MenuPrint(outputString);         
	
	 
   
   firstEnter = 0;   
}


/** Function to setup each individual channel */
void ChannelSettings(void* data)
{
   uint8_t* input = 0;
   uint8_t	printReading = 0;
   uint8_t outputString[21];
	float32_t sample; 
	uint8_t chGain;
   static uint32_t ADCValue;
   static int8_t SelectedChannel = 0;
   static int8_t SelectedGain = 0;
   
   input = data;
   
   if( input[1] == 0xFF )
   {
		printReading = 1;	
	}

   if( firstEnter == 0 )
   {      
      switch( *input )
      {
			case KB_UP:
         case KP_UP:
				if( ++SelectedChannel > (SENSOR_COUNT - 1) )
				{
					SelectedChannel = 0;	
				}       
         break;
         
			case KB_DOWN:
         case KP_DOWN:
				if( --SelectedChannel < 0 )
				{
					SelectedChannel = (SENSOR_COUNT - 1);	
				}       
         break;
         
         /* Setting Modifiers */
         case 'A':
         case KP_A:
				SensorToggle(SelectedChannel);
            break;
         
         case 'B':
         case KP_B:
         	SensorTypeToggle(SelectedChannel);
         	break;
         
         case 'C':
			case KP_C:
				SelectedGain = SensorGetGain(SelectedChannel);
				if( ++SelectedGain > (GAIN_COUNT - 1) )
				{
					SelectedGain = 0;
				}
				SensorSetGain( SelectedChannel, SelectedGain);				
				break;
         
         /* Menu Function exit routine */
         case KB_BACK:
         case KP_BACK:
          
            MenuSetInput(KP_BACK);
            stateMachine(currentState);
            MenuSetInput(0);
            return;
            
            break;
            
         default:
            break;
      }
      
   }
   
   firstEnter = 0;	
	
	/* Indicate the channel selected */
	uint8toa(SelectedChannel + 1, outputString);
	MenuPrint_P(PSTR("Channel: "));
	MenuPrint(outputString);          
   MenuNewLine();	
   
   /* Print the status (only if we aren't going to print the reading)*/
   if( printReading == 0)
   {
		MenuPrint_P(PSTR("Status: "));   
		if( SensorGetState(SelectedChannel) == SENSOR_ON )
		{
			MenuPrint_P( PSTR("On") );
		}
		else
		{
			MenuPrint_P( PSTR("Off") );		
		}
		MenuNewLine();	
	}
	else
	{
		MenuPrint_P(PSTR("= "));   
		
		GS_Channel(SelectedChannel + 1);
		
  		chGain = SensorGetGain(SelectedChannel);
      GS_GainSel( pgm_read_byte( &GS_GAIN[chGain]) );  		

		ADS1213_Reset();                 
   	/** Condition the data from the ADC */
		ADCValue = ADS1213_GetResult();
		ADCValue -= GAIN_OFFSETS[chGain];
	   sample = SensorCondition(ADCValue, chGain); 
		/* Print out the Voltage / Temperature */
		if( SensorGetType(SelectedChannel) == SENSOR_TEMP )
		{
			sample.FP = (sample.FP * SC_K_SEEBECK_COEFF_INV);
			/** Ambient Temperature */
			sample.FP = sample.FP + ambientTemperature + SC_K_TEMP_OFFSET;
		}	
		
		printFloat(sample.FP, outputString);
		MenuPrint(outputString);
		
		if( SensorGetType(SelectedChannel) == SENSOR_TEMP )
		{
			MenuPrint_P(PSTR(" ßC")); 	
		}
		else
		{
			MenuPrint_P(PSTR(" V")); 	
		}
		
		MenuNewLine();			
		
	}

   /* Indicate the Channel Type */
	MenuPrint_P(PSTR("Input: "));   
	if( SensorGetType(SelectedChannel) == SENSOR_VOLTAGE )
	{
		MenuPrint_P( PSTR("Voltage") );
	}
	else
	{
		MenuPrint_P( PSTR("Temperature") );		
	}
	MenuNewLine();	   
	
	/* Display the channel 'gain' */
	uint8toa(SensorGetGain(SelectedChannel), outputString);
	MenuPrint_P(PSTR("Gain: "));
	MenuPrint(outputString);          
   MenuNewLine();
}


void BeginRecording(void* data)
{
	uint8_t outputString[10];
	uint8_t* input = (uint8_t*)data;
	
	MenuPrint_P( PSTR("Press A to update") );
	MenuNewLine();		
	MenuPrint_P( PSTR("Press C to stop") );

	
	if( firstEnter == 1)
	{
		/* Write file to SD Card */
		MM_CreateRecording((uint8_t*)"NewFile");
		
		/* reset the data file name to prevent overwriting */
		
		/* Write the sensor states */
		MM_Write32( SensorState );
		MM_Write32( SensorType );
		
		/* Take a sample */
		SC_Sample();
		
		/* Reset Timers */
		SC_INTLongDelay.timerCounter = 0;
		SC_MasterTimer.timerCounter = 0;
		
		/* Enable the correct timers */
		if( SC_GetMode() == SC_SAMPLE_EXTERNAL )
		{
			DS1305_AlarmControl(DS1305_RTC0, 1);
			DS1305_SetAlarm( (uint8_t*)SC_RTC0_CONFIG , DS1305_RTC0);
			SC_MasterTimer.timerEnable = TIMER_DISABLE;
			SC_INTLongDelay.timerEnable = TIMER_ENABLE;
		}
		else
		{
			SC_MasterTimer.timerEnable = 1;
			DS1305_AlarmControl(DS1305_RTC0, 0);
			SC_MasterTimer.timerEnable = TIMER_ENABLE;
			SC_INTLongDelay.timerEnable = TIMER_DISABLE;
		}
	}
	
	firstEnter = 0;

	switch( *input )
	{
		case KP_A:
			/* Print the number of samples and sample rate */
			MenuNewLine();
			MenuPrint_P( PSTR("Sample Count: ") );
			ultoa(MasterDataRecord.sampleCount, (char*)outputString, 10);
			MenuPrint(outputString);
			MenuNewLine();
			PrintSampleRate();
		break;
		
		case KP_C:
			/* Disable Timers */
			SC_MasterTimer.timerEnable = TIMER_DISABLE;
			SC_INTLongDelay.timerEnable = TIMER_DISABLE;
			
			/* Write the stop byte */
			MM_Write(0xFF);
			/* Write rest of cached data */
			MM_Sync();
			
			DS1305_AlarmControl(DS1305_RTC0, 0);			
			
			
			
			/* Go back up one menu */   
   		MenuSetInput(KB_BACK);
  			stateMachine(currentState);
  			MenuSetInput(0);
  			MenuReset();
  			executeState(currentState);
		break;
		
	}
}





void MenuSetTime(void* data)
{
	static int8_t newTime[MKTIME_HOURS+1];
	uint8_t returnVal;
	
	/* Get the time */
	if( firstEnter == 1)
	{
		DS1305_GetTime(DS1305_TimeDate_config);
		newTime[MKTIME_SECONDS] = BCD2DEC(DS1305_TimeDate_config[SECONDS]);
		newTime[MKTIME_MINUTES] = BCD2DEC(DS1305_TimeDate_config[MINUTES]);	
		newTime[MKTIME_HOURS] = BCD2DEC((DS1305_TimeDate_config[HOURS] & 0x3F));
	}
	firstEnter = 0;
	returnVal = MakeTime(data, newTime);
	
	switch( returnVal )
	{
		case MKTIME_CANCEL:
			/* Exit and cancel */
			UI_LCD_ClearCursor();
			MenuSetInput(KB_BACK);
   		stateMachine(currentState);
		break;			
			
		case MKTIME_PENDING:
			return;
		break;
			
		case MKTIME_CONFIRMED:
			/* Exit and save */
			DS1305_TimeDate_config[SECONDS] = DEC2BCD(newTime[MKTIME_SECONDS]);
			DS1305_TimeDate_config[MINUTES] = DEC2BCD(newTime[MKTIME_MINUTES]);
			DS1305_TimeDate_config[HOURS] = DEC2BCD(newTime[MKTIME_HOURS]);
						
			DS1305_SetTime(DS1305_TimeDate_config);
			
			
			UI_LCD_ClearCursor();			
			MenuSetInput(KB_BACK);
   		stateMachine(currentState);
			
		break;

		default:
		break;
	}
}


/** Allow the user to generate a time stamp based on user input */
uint8_t MakeTime(void* data, int8_t* timeComponent )
{
	
   uint8_t outputString[11];
   uint8_t buffer[4];
   uint8_t* input = (uint8_t*)data;
   int8_t i;
   
	const uint8_t timeLimits[MKTIME_HOURS+1] = {SECONDS_MAX, MINUTES_MAX, HOURS_MAX};
	/* Either SECONDS, MINUTE or HOUR */
	static int8_t selectedComponent;

   if( firstEnter == 0 )
   {     
      switch( *input )
      {
         /* Increment selected time */
         case 'q':
         case KP_A:
            ++timeComponent[selectedComponent];   
         break;
            
         /* Deccrement selected time */  
         case 'a':                   
         case KP_B:
            --timeComponent[selectedComponent]; 
         break;
  
         /* Time Component increment function */
         case KB_BACK:
         case KP_BACK:
				if( ++selectedComponent > MKTIME_HOURS )
				{
					selectedComponent = 	MKTIME_HOURS;
					return MKTIME_CANCEL;
				}

         break;
         
         /* Time Component decrement function and exit*/
         case KB_ENTER:
         case KP_ENTER:
         	if(--selectedComponent < 0)
         	{
					selectedComponent = 0;
				}
         break;
         
         case 'c':
         case KP_C:
               /* Return the time here */
               return MKTIME_CONFIRMED;
         break;
               
         default:
         break;     
      }
   }
   
   outputString[0] = 0;
   
   /* Ensure Seconds to Hours are within their limits */
   for( i = MKTIME_HOURS; i >= 0; i--)
   {
		if( timeComponent[i] > timeLimits[i] )
		{
			timeComponent[i] = 0;		
		}
		
		if( timeComponent[i] < 0 )
		{
			timeComponent[i] = timeLimits[i];
		}
		
		/* Write them to a string */
		uint8toa(timeComponent[i], buffer);
		
		if( timeComponent[i] < 10)
		{
			strcat( (char*)outputString, "0" );	
		}
						
		strcat( (char*)outputString, (const char*)buffer );
		strcat( (char*)outputString, ":" );
	}
	
   firstEnter = 0;
   
   /* Get rid of the last ':' */
   outputString[8] = 0;
   
   /* Enable Blinking Cursor */
   UI_LCD_SetCursor();
   
	/* Print instructions */
	MenuPrint_P( PSTR("Press 'C' to save") );
	MenuNewLine();
	MenuPrint_P( PSTR("24 Hour Mode") );	
	MenuNewLine();
	/* Move to centre of screen and print edited time */
	UI_LCD_Pos(2, 5);
	MenuPrint( outputString );
	   
   /* Set position of cursor to the editing component */
   UI_LCD_Pos(2, 12 - (3*selectedComponent) );
   
   

	return MKTIME_PENDING;
   
}


void SetSamplingRate(void* data)
{
	uint8_t* input = (uint8_t*)data;
	static uint8_t firstEnter = 1;
	
		
	if( firstEnter != 1 )
   {     
		stateMachine( currentState );
      switch( *input )
      {
			case KB_BACK:
			case KP_BACK:
				firstEnter = 1;
				return;
			break;
				
			case KB_ENTER:
			case KP_ENTER:
				MenuReset();
		   	executeState(currentState);
				firstEnter = 1;
				return;	
			break;

			default:
			break;
		}	
	}
	
	MenuReset();
	firstEnter = 0;
   PrintSampleRate();
   MenuNewLine(); 
}


void PrintSampleRate(void)
{
	uint8_t outputString[6];
	
	MenuPrint_P( PSTR("S. Rate: ") );	
   if( SC_GetMode() == SC_SAMPLE_INTERNAL)
   {
		uint8toa( SC_GetShortRate(), outputString);
		MenuPrint(outputString);
      MenuPrint_P( PSTR("0 ms"));
   }
   else
   {
		uint16toa(SC_GetLongRate(), (char*)outputString, 0);
		MenuPrint(outputString);
      MenuPrint_P( PSTR(" s"));      
   }
}

void SetShortRate(void* data)
{
	uint8_t* input = (uint8_t*)data;
	static uint8_t	tenMillisecDelay;
	uint8_t outputString[4];
	
	if( firstEnter == 1)
	{
		tenMillisecDelay = SC_GetShortRate();
	}
	
	
   if( firstEnter == 0 )
   {     
      switch( *input )
      {
         /* Increment msec delay */
         case 'q':
         case KP_A:
            ++tenMillisecDelay;   
         break;
            
         /* Deccrement msec delay */  
         case 'a':                   
         case KP_B:
            --tenMillisecDelay; 
         break;
  
         /* Time Component increment function */
         case KB_BACK:
         case KP_BACK:
				/* Go back up one menu */   
   			MenuSetInput(KB_BACK);
  				stateMachine(currentState);
  				MenuSetInput(0);
  				executeState(currentState);
  				return;
  				
         break;
                        
         default:
         break;     
      }
   }
   
   firstEnter = 0;
   
   MenuPrint_P( PSTR("Millisecond Delay:"));
	MenuNewLine();
	
	uint8toa( tenMillisecDelay, outputString);
	MenuPrint(outputString);
	MenuPrint_P( PSTR("0 milliseconds") );
	      
	SC_SetSamplingRate( tenMillisecDelay );

}

/* Instead of get time, it is get Alarm */
void SetLongRate(void* data)
{
	static int8_t newAlarm[DS1305_SIZEOFALARM];
	uint16_t totalDelay;
	uint8_t returnVal;
	
	/* Get the current alarm time */
	MenuReset();
	UI_LCD_Pos(3, 0);
	MenuPrint_P( PSTR("Max Delay: 18 Hours") );
	UI_LCD_Home();

	firstEnter = 0;
	returnVal = MakeTime(data, newAlarm);
	
	switch( returnVal )
	{
		case MKTIME_CANCEL:
			/* Exit and cancel */
			UI_LCD_ClearCursor();
			MenuSetInput(KB_BACK);
   		stateMachine(currentState);
			MenuSetInput(0);
			executeState(currentState);
		break;			
			
		case MKTIME_PENDING:
		break;
			
		case MKTIME_CONFIRMED:
			/* Exit and save */
			totalDelay = newAlarm[SECONDS] 
							 + newAlarm[MINUTES] * 60
							 + newAlarm[HOURS] * 3600;
			
			SC_SetSamplingRate_Long( totalDelay ); 
			UI_LCD_ClearCursor();			
			MenuSetInput(KB_BACK);
   		stateMachine(currentState);
			MenuSetInput(0);
  			executeState(currentState);
		break;

		default:
		break;
	}	
	
}

void MenuDisplayMode(void* data)
{
	uint8_t* input = (uint8_t*)data;
	static uint8_t firstEnter = 1;
	
	if( firstEnter != 1 )
   {     
		stateMachine( currentState );	
   	
      switch( *input )
      {
			case KB_BACK:
			case KP_BACK:
				firstEnter = 1;
				MenuReset();
				return;
			break;
				
			case KB_ENTER:
			case KP_ENTER:
		   	executeState(currentState);
				firstEnter = 1;	
			break;

			default:
			break;
		}	
	}
	
	firstEnter = 0;
   MenuPrint_P( PSTR("The current mode is:") );
   MenuNewLine();		
   if( uartMenu == MENU_LCD)
   {
      MenuPrint_P(MT_LCD_MODE);
   }
   else
   {
      MenuPrint_P(MT_UART_MODE);      
   }
   MenuNewLine();   
}

void MenuSetUartMode(void* data)
{
   /* Update the display mode */   
   MenuSetDisplay(MENU_UART);   
   uartMenu = MENU_UART;
   
   /* Go back up one menu */   
   MenuSetInput(KB_BACK);
   stateMachine(currentState);
}

void MenuSetLCDMode(void* data)
{
   /* Update the display mode */
   MenuSetDisplay(MENU_LCD);   
   uartMenu = MENU_LCD;
   /* Go back up one menu */
   MenuSetInput(KB_BACK); 
   stateMachine(currentState);
}


void MediumSelect(void* data)
{
	uint8_t* input = (uint8_t*)data;
	static uint8_t firstEnter = 1;
	
	if( firstEnter != 1 )
   {     
		stateMachine( currentState );	
   	
      switch( *input )
      {
			case KB_BACK:
			case KP_BACK:
				firstEnter = 1;
				MenuReset();
				return;
			break;
				
			case KB_ENTER:
			case KP_ENTER:
		   	executeState(currentState);
				firstEnter = 1;	
			break;

			default:
			break;
		}	
	}
	
	firstEnter = 0;
   MenuPrint_P( PSTR("Selected Media:") );
   MenuNewLine();		
   if( MM_GetMemoryType() == MM_SDCARD)
   {
		if( hasSDCard == 0 )
		{
			MenuPrint_P(PSTR("No SD Card"));	
		}
		else
		{
      	MenuPrint_P(MT_SDCARD);
		}
   }
   else
   {
      MenuPrint_P(MT_EEPROM);      
   }
   MenuNewLine();   
}

void MediumEEPROM(void* data)
{
   /** Set the medium */   
   MM_SetMemoryType(MM_EEPROM);   
  
   /* Go back up one menu */   
   MenuSetInput(KB_BACK);
   stateMachine(currentState);
}

void MediumSDCard(void* data)
{
   /** Set the medium */   
   MM_SetMemoryType(MM_SDCARD);   
  
   /* Go back up one menu */   
   MenuSetInput(KB_BACK);
   stateMachine(currentState);
}


/** Just Perform a data dump in relation to the number of samples... */
void ReadRecording(void* data)
{
	uint16_t cnt;
	uint32_t i;
	uint8_t remainingBytes;
	uint8_t buffer[MM_BUFFER_SIZE + 1]; /// +1 for NULL byte 
	uint16_t fileSize;
	FIL inFile;
	EE_AddressStruct inData = {0, 0};

	if( MM_GetMemoryType() == MM_SDCARD )
	{
		if( f_open(&inFile, (char*)"newFile", FA_READ) )
		{
			uartTxString_P( PSTR("Open Failed!") );	
		}	
		fileSize = inFile.fsize;
			
		for( i = 0; i < (uint32_t)(fileSize / MM_BUFFER_SIZE); i++ )
		{
			buffer[0] = 0;
			if( f_read(&inFile, buffer, MM_BUFFER_SIZE, &cnt) )
			{
				uartTxString_P( PSTR("Read Failed!") );			
			}
			uartTxDump( buffer, MM_BUFFER_SIZE );
		}
		
		remainingBytes = fileSize - (i * MM_BUFFER_SIZE);
		
		if( f_read(&inFile, buffer, remainingBytes, &cnt) )
		{
			uartTxString_P( PSTR("Read Failed!") );			
		}
		uartTxDump( buffer, remainingBytes );
	}
	else /// EEPROM Mode 
	{
		
		for( i = 0; 
			  i < (MasterDataRecord.eepromFile.EE_Address / MM_BUFFER_SIZE);
			  i++)
		{
			/** EEPROM Read */
			serialEE_ReadBlock( buffer, MM_BUFFER_SIZE, &inData);
			/* Update the eeprom address */
			inData.EE_Address += MM_BUFFER_SIZE;
			
			uartTxDump( buffer, MM_BUFFER_SIZE );
		}
		
		remainingBytes = MasterDataRecord.eepromFile.EE_Address - (i * MM_BUFFER_SIZE);
		serialEE_ReadBlock(  buffer, remainingBytes, &inData);
		uartTxDump( buffer, remainingBytes );
	}
	
	
   /* Go back up one menu */   
   MenuSetInput(KB_BACK);
   stateMachine(currentState);	
	
}



void Calibrate(void* data)
{
	
	uint8_t* input = (uint8_t*)data;
	
	MenuPrint_P( PSTR("Please short CH 4"));
	MenuNewLine();
	MenuPrint_P( PSTR("and press C"));	
	
	
	static uint8_t dataCalib[4];
	uint32_t* dataPtr = &dataCalib[0];
	char outputString[20];

	if( firstEnter == 0 )
	{
		if( *input == KP_C )
		{
			GS_Channel(CALIBRATION_CHANNEL);
			GS_GainSel( pgm_read_byte( &GS_GAIN[CALIBRATION_GAIN])  );
			_delay_ms(200);
			ADS1213_OpMode(ADS1213_SELF_CALIB);
			//ADS1213_PsuedoCalib();
			
			ADS1213_ReadInstruction(ADS1213_OFFSET_CAL_REG_2, dataCalib, 3);


			dataCalib[3] = dataCalib[0];
			dataCalib[0] = dataCalib[2];
			dataCalib[2] = dataCalib[3];

			dataCalib[3] = 0;

			ltoa( *dataPtr, outputString, 10);
		    uartTxString( (uint8_t*)"Calibration Offset = ");
		    uartTxString(outputString);
		    uartNewLine();


			ADS1213_ReadInstruction(ADS1213_FULLSCALE_CAL_REG_2, &dataCalib[2], 1);
			ADS1213_ReadInstruction(ADS1213_FULLSCALE_CAL_REG_1, &dataCalib[1], 1);
			ADS1213_ReadInstruction(ADS1213_FULLSCALE_CAL_REG_0, &dataCalib[0], 1);

			dataCalib[3] = 0;

			ltoa( *dataPtr, outputString, 10);
		    uartTxString( (uint8_t*)"FS Calib = ");
		    uartTxString(outputString);
		    uartNewLine();

   		/* Go back up one menu */   
   		MenuSetInput(KB_BACK);
   		stateMachine(currentState);				
			MenuReset();	
		}
	}
	
	firstEnter = 0;
}



void SetDecimation(void* data)
{
	uint8_t* input = (uint8_t*)data;
	uint8_t outputString[4];
	
	
   if( firstEnter == 0 )
   {     
      switch( *input )
      {
         /* Increment DCR */
         case 'q':
         case KP_A:
            ADS1213_DecimationRatio += 100;
				if( ADS1213_DecimationRatio > ADS1213_MAX_DCR)
				{
					ADS1213_DecimationRatio = ADS1213_MIN_DCR;
				}
				   
         break;
            
         /* Deccrement DCR  */  
         case 'a':                   
         case KP_B:
            ADS1213_DecimationRatio -= 100;
				if( ADS1213_DecimationRatio < ADS1213_MIN_DCR)
				{
					ADS1213_DecimationRatio = ADS1213_MAX_DCR;	
				}
         break;
         
         
         case KP_C:
            ADS1213_DecimationRatio += 10;
				if( ADS1213_DecimationRatio < ADS1213_MIN_DCR)
				{
					ADS1213_DecimationRatio = ADS1213_MAX_DCR;	
				}
         break;         
  
         /* Time Component increment function */
         case KB_BACK:
         case KP_BACK:
				/* Go back up one menu */   
   			MenuSetInput(KB_BACK);
  				stateMachine(currentState);
  				MenuSetInput(0);
  				executeState(currentState);
  				return;
  				
         break;
                        
         default:
         break;     
      }
   }
   
   firstEnter = 0;
   
   MenuPrint_P( PSTR("Decimation Ratio:"));
	MenuNewLine();
	
	uint16toa( ADS1213_DecimationRatio, (char*)outputString, 0);
	MenuPrint(outputString);
	      
	
	ADS1213_CS_PORT &= ~(1 << ADS1213_CS_PIN);
   _delay_us(10);  	     
   ADS1213_TxByte( (1 << ADS1213_MB0) | (1 << ADS1213_A2) | (1 << ADS1213_A1) );  

   ADS1213_TxByte( (1<<ADS1213_SF2) |  (ADS1213_DecimationRatio >> 8) );
   /* Have a data rate of 2kHz */
   ADS1213_TxByte( ADS1213_DecimationRatio & 0xFF);

   ADS1213_CS_PORT |= (1 << ADS1213_CS_PIN);        
   _delay_us(10);     	
	
}

void printAmbientTemperature(void* data)
{
	uint8_t* input = (uint8_t*)data;
	uint8_t outputString[10];
	
	MenuPrint_P( PSTR("Ambient Temperature:"));
	MenuNewLine();

	ambientTemperature =	TMP123_GetTempFP( TMP123_GetTemp() );
			
	printFloat(ambientTemperature, outputString);
	MenuPrint( outputString );
	MenuPrint_P(PSTR(" ßC")); 	
	
	
	if( firstEnter == 0 )
	{
		if( *input == KP_BACK )
		{
   		/* Go back up one menu */   
   		MenuSetInput(KB_BACK);
   		stateMachine(currentState);				
			MenuReset();	
		}
	}
	
	firstEnter = 0;
}

