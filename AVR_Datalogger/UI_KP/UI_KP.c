

#include "UI/UI.h"
#include "i2c/i2c.h"
#include "UI_KP.h"
#include "hardUart/hardUart.h"

#define ROM_LOC PROGMEM

static const uint8_t ROM_LOC KP_Button[] = {KP_2, KP_8, KP_5, KP_HASH, 
                                  KP_4, KP_6, KP_A, KP_B, 
                                  KP_C, KP_D, KP_1, KP_3,
                                  KP_7, KP_9, KP_0, KP_STAR};

void UI_KP_Init(void)
{
   /* All ports are by default inputs with no pullups */

	/* Set all UI_ROWS outputs */
	i2cAddress(UI_MAX7300_ADDRESS, TW_WRITE);	
	i2cTransmit(MAX7300_DDRB1);
	i2cTransmit(0x55);
	
	/* UI_COLS to Inputs and ROW0 to output*/
	i2cTransmit(0xA9);

   /* Enable Pullups on RTC0/RTC1 */
   /* Set P31 to an output and COL0 to an input*/
	i2cTransmit(0x7E);
	i2cStop();	

   /* Set UI_Rows to to output LOW */
   UI_SetRegister(UI_ROW_PORT, 0x00);
   
}

uint8_t UI_KP_GetPress(void)
{
   
   uint8_t ColResult;
   uint8_t RowResult;
   uint8_t KPResult;
   
	ColResult = (UI_ReadRegister(MAX7300_PORTINT) >> 1) & (0x0F);
   
   /* Set rows to inputs with no PULLUP and columns to outputs*/
	i2cAddress(UI_MAX7300_ADDRESS, TW_WRITE);     
 	i2cTransmit(MAX7300_DDRB1);
	i2cTransmit(0xAA); 
	i2cTransmit(0xAA);
	i2cStop();   

   /* Set Columns to outputs */
	i2cAddress(UI_MAX7300_ADDRESS, TW_WRITE);     
 	i2cTransmit(MAX7300_DDRB2);
	i2cTransmit(0x56); 
	i2cTransmit(0x7D);
	i2cStop();
	
	/* Set all columns to Output High and Bring INT0 low*/
	UI_SetRegister(UI_COL_PORT, 0x0F);
   
   /* Get the row result */
	RowResult = ~(UI_ReadRegister(UI_ROW_PORT)) & (0x0F);  

   KPResult = (ColResult << 4) | RowResult;
   
  
   /* Reset the buttons to original state */
   UI_KP_Init();
   
   return KPResult;
   
}

