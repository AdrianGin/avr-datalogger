#include <avr/io.h>
#include <stdlib.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <Util/delay.h>
#include <avr/sleep.h>

#include "I2C/i2c.h"
#include "SPI/spi.h"
#include "hardUart/hardUart.h"
#include "SampleCtrl/SampleCtrl.h"
#include "UI_KP/UI_KP.h"
#include "UI_LCD/UI_LCD.h"
#include "UI/UI.h"
#include "Menu/Menu.h"
#include "GainSensor/SensorControl.h"
#include "GainSensor/GainControl.h"
#include "ADS1213/ads1213.h"
#include "DS1305/ds1305.h"
#include "RTC/RTCPrint.h"
#include "SD_MMC/sd.h"
#include "uartTerm/uartInput.h"
#include "MemMan/memman.h"
#include "TMP123/tmp123.h"
#include "mmculib/uint16toa.h"
#include "UserFunctions/userFunctions.h"

/* INT0 interrupt is connected to an Active High Interrupt 
 * Thus set MCUCR |= (0x03 << ISC00) */

/* Todo: Menu Handling and Button Presses + RTC interrupts (Low Prio) */
/* Write functions to get and return Sensor data */

extern uint8_t currentState;

/* Waking up from Power Down requires 6Clock Cycles */
float ambientTemperature;
uint16_t sampleSetting;
uint8_t channel = 1;
uint8_t gain = 0;
uint8_t uartMenu = 0;
uint8_t hasSDCard;
FATFS filesys;

void printSample(void);
void Channel_Setup(void);

/* To do, MMC add #conditional includes
 * MAX7300 routines.
 */
int main(void)
{
	uint8_t i;

	for (i = 0; i < 50; i++)
	{
		_delay_ms(30);
	}

	ADS1213_POR();

	SPI_Init();
	SPI_RxByte();
	SPI_RxByte();

	MMC_CS_DDR |= (1 << MMC_CS_PIN);
	MMC_CS_PORT |= (1 << MMC_CS_PIN);

	/* Good for 250kbit */
	uartInit(7, 1);

	SD_Shutdown();

	UCSRB |= (1 << RXCIE);

	i2cInit(2, 0);

	GS_Init();
	TMP123_Init();
	UI_Activate();
	UI_KP_Init();

	UI_LCD_HWInit();
	UI_LCD_Shutdown();
	_delay_ms(10);

	UI_LCD_Activate();
	UI_LCD_Init();

	MenuSetDisplay(MENU_LCD);

	/* Enable Keypad Presses Disable INT0 and use INT2 */
	UI_LCD_SetData();
	DDRB &= ~(1 << PB2);
	MCUCSR |= (1 << ISC2);
	GICR |= (1 << INT2);

	/* Sample Control */
	SC_Init();
	SC_EnableTimer();

	/* Menu Setup */
	MenuSetInput(0);
	/* Reprint Menu */
	MenuUpdate();

	/* Setup ADC, perform offset calibration */
	ADS1213_Init();

	/** Initialise SD Card */
	if (SD_Init() == SD_SUCCESS)
	{
		/* Mount the drive */
		f_mount(0, &filesys);
		uartTxString_P(PSTR("SD Card Initialised!"));
		MMC_CS_PORT |= (1 << MMC_CS_PIN);
		SPCR |= (1 << CPHA) | (1 << SPR1);
		hasSDCard = 1;
	}
	else
	{
		/* Release and return clock phase and speed back to default */
		MMC_CS_PORT |= (1 << MMC_CS_PIN);
		SPCR |= (1 << CPHA) | (1 << SPR1);

		uartTxString_P(PSTR("SD Card Failed!"));
		hasSDCard = 0;
	}

	/* Write to the DS1305 The time and date */
	/* Max frequency is 600kHz */
	DS1305_Init();

	//DS1305_SetTime(DS1305_TimeDate_config);

	ambientTemperature = TMP123_GetTempFP(TMP123_GetTemp());
	UI_SetRegister(MAX7300_CONFIG,
			(1 << MAX7300_SHUTDOWN_CONTROL) | (1 << MAX7300_TRANSITION_ENABLE));

	GS_Channel(CALIBRATION_CHANNEL);
	GS_GainSel(pgm_read_byte(&GS_GAIN[CALIBRATION_GAIN]));
	ADS1213_PsuedoCalib();

	sei();

	while (1)
	{
		/* Need to reset Interrupt */
		set_sleep_mode(SLEEP_MODE_IDLE);
		sleep_enable()
		;
		sleep_cpu()
		;
		sleep_disable()
		;
		sei();
	}

	return 0;

}

/* To echo the receiver buffer*/
ISR(USART_RXC_vect)
{
	cli();
	uint8_t rcvdByte = 0;

	rcvdByte = UDR;

	/* Update the menu if we are in UART mode */
	if (uartMenu)
	{
		MenuSetInput(rcvdByte);
		MenuUpdate();
	}

	/* Use the UART Terminal Interface */

	else
	{
		switch_uart_input(rcvdByte);
		return;
	}

}

/* Counts number of SC_Compare_rate us. */
/* Perhaps use a faster FPU speed ? */
/* For 1MHZ With a presacler of 8, 125 == 1 msec so this is a 1msec counter */
/* For 8MHZ               "     32, 250 = 1 msec */
ISR(TIMER2_COMP_vect)
{
	cli();
	static uint8_t counter_ms;

	/* Example */
	/* Happens every 10 seconds, max seconds = 25.5secs
	 * Although we can use a uint16_t variable to obtain a 6502.5 sec max */

	/* Restart after 2 seconds */
	static SoftTimer_16 temperatureUpdate =
	{ 1000, 0, 1 };

	counter_ms++;

	if (counter_ms == 10 * SC_MILLISECOND)
	{
		counter_ms = 0;

		/* Clock the Master Timer at a 10ms Resolution */
		temperatureUpdate.timerCounter++;
		SC_MasterTimer.timerCounter++;

		/* Functions which occur every xx*10msecs happen here */
		if (SC_MasterTimer.timerCounter == SC_MasterTimer.timeCompare
				&& SC_MasterTimer.timerEnable)
		{
			/* Take a sample */
			SC_Sample();
			/* Reset the timer */
			SC_MasterTimer.timerCounter = 0;
		}

		if (temperatureUpdate.timerCounter == temperatureUpdate.timeCompare)
		{
			ambientTemperature = TMP123_GetTempFP(TMP123_GetTemp());
			temperatureUpdate.timerCounter = 0;
		}

	}
}

/* This is used to parse the KP inputs and RTC inputs */
/* It is important to handle RTC inputs first */
ISR(INT2_vect)
{

	/* Should have a debounce counter... */
	cli();

	uint8_t IntResult;
	uint8_t outputString[4];

	IntResult = ~(UI_ReadRegister(MAX7300_PORTINT) >> UI_RTC_INT0) & (0x03);
	/* Need to reset Interrupt */
	UI_SetRegister(UI_INTERRUPT, 0);
	UI_SetRegister(MAX7300_CONFIG, (1 << MAX7300_SHUTDOWN_CONTROL));

	/* Check for RTC interrupt */
	switch (IntResult)
	{
	/* RTC_INT0 Triggered */
	case 1:
		/* INT0 Handler */
		/* Reset the RTC0 IRQ Flag */
		DS1305_ReadByte(DS1305_A0_SECS);
		uartTxString((uint8_t*) "RTC INT0 Triggered!");
		uint16toa(SC_INTLongDelay.timerCounter, (char*) outputString, 0);
		uartTxString(outputString);

		if ((SC_INTLongDelay.timerEnable
				&& ++SC_INTLongDelay.timerCounter == SC_INTLongDelay.timeCompare))
		{
			/* Take a sample */
			SC_Sample();
			SC_INTLongDelay.timerCounter = 0;
		}
		break;

		/* RTC_INT1 Triggered */
	case 2:
		/* INT1 Handler */
		uartTxString((uint8_t*) "RTC INT1 Triggered!");
		break;

		/* Both triggered */
	case 3:
		/* INT1 has priority */
		//uartTxString((uint8_t*)"BOTH RTC INTS Triggered!");
		/* INT1 Handler */
		/* INT0 Handler */
		break;

		/* Interrupt must have been from a Button press then */
	default:
		IntResult = UI_KP_GetPress();

		if (IntResult == KP_D)
		{
			LCD_BL_PORT ^= (1 << LCD_BL_PIN);
		}

		if (IntResult != KP_INVALID)
		{
			MenuSetInput(IntResult);
			MenuUpdate();
		}
		break;
	}

	/* Set the M-bit in the UI Register */
	UI_Activate();
	sei();
}

