/** \file Menu.h
	 \brief Menu Machine for AVR
*/

/**	 

	\ingroup avr_datalogger
 	\defgroup menu Menu State Machine
 	\code #include "menu.h" \endcode

 	 
	 \par Description
	Implements a menu function.
 	 
*/
//@{


#ifndef	_MENU_ROUTINES
#define	_MENU_ROUTINES

#define MENU_DEBUG 0

#if MENU_DEBUG == 0
#include <avr/pgmspace.h>
#define PRINT_FUNC   UI_LCD_String_P
#define MENU_TEXT    char PROGMEM
#endif

#if MENU_DEBUG == 1
#include "stdint.h"
#define PSTR(x) (x)
#define PRINT_FUNC   printf
#define MENU_TEXT    char
#endif

#define NO_STATE     0
#define INVALID_STATE 255
#define INVALID_SEQUENCE 255

#define MAIN_MENU    ST_MAIN
#define MENU_LCD  0
#define MENU_UART 1



#define WINDOW_SIZE 3

#if MENU_DEBUG == 1

#define KP_1   '1'
#define KP_2   '2'
#define KP_3   '3'
#define KP_4   '4'
#define KP_5   '5'
#define KP_6   '6'
#define KP_7   '7'
#define KP_8   '8'
#define KP_9   '9'



#endif

#define KB_UP '8'
#define KB_DOWN '2'
#define KB_ENTER '6'
#define KB_BACK  '4'

#define KP_UP  KP_2
#define KP_DOWN KP_8
#define KP_ENTER KP_6
#define KP_BACK  KP_4

typedef struct
{
   uint8_t  parent;
   uint8_t  child;
   uint8_t  sequence; /// Effectively, which line of the LCD the item is listed on.
   
} menu_list;


typedef struct
{
   uint8_t  menu_item;
   
#if MENU_DEBUG == 1   
   const char* descriptor;
#else
   PGM_P descriptor;   
#endif
   void (*function)(void* data);

} menu_data;

/* Menu Texts */
extern const MENU_TEXT  MT_NEW_RECORDING[];
extern const MENU_TEXT  MT_VIEW_CHANNELS[];
extern const MENU_TEXT  MT_STATISTICS[];
extern const MENU_TEXT  MT_SD_CARD_SIZE[];
extern const MENU_TEXT  MT_TOTAL_SAMPLES[];
extern const MENU_TEXT  MT_UPTIME[];
extern const MENU_TEXT  MT_DATA_DOWNLOAD[];
extern const MENU_TEXT  MT_OPTIONS[];
extern const MENU_TEXT  MT_SET_TIME[];
extern const MENU_TEXT  MT_CALIBRATE[];
extern const MENU_TEXT  MT_FORMAT_SD[];
extern const MENU_TEXT  MT_RESET[];
extern const MENU_TEXT  MT_VERSION_INFO[];
extern const MENU_TEXT  MT_DISPLAY_MODE[];
extern const MENU_TEXT  MT_UART_MODE[];
extern const MENU_TEXT  MT_LCD_MODE[];
extern const MENU_TEXT  MT_EEPROM[];
extern const MENU_TEXT  MT_SDCARD[];


extern uint8_t currentState;
extern uint8_t RowPosition;
extern uint8_t upperLimit;
extern uint8_t lowerLimit;
extern uint8_t firstEnter;
extern uint8_t selectedItem;
/** General Menu Functions */
void MenuSetDisplay(uint8_t display);
void (*MenuPrint)(uint8_t* string);
/* For a string residing in FLASH */
void (*MenuPrint_P)(const char* string);
void (*MenuNewLine)(void);
void (*MenuReset)(void);




void LCD_NewLine(void);
void LCD_Reset(void);

/* Menu Uart Functions */
void MenuUart_NewLine(void);
void MenuUartTxString(uint8_t* string);
void MenuUartTxString_P(const char* string);
void MenuUart_Reset(void);


void MenuSetDisplay(uint8_t display);

void MenuSetInput(uint8_t NewInput);

void MenuUpdate(void);
char* MenuDescriptor(uint8_t menuItem);
void stateMachine(uint8_t CurrentState);
uint8_t GetMenuState(uint8_t state, uint8_t Sequence);
uint8_t GetParent(uint8_t state);
uint8_t SubItems(uint8_t state);
uint8_t GetIndex(uint8_t parent);
void executeState(uint8_t state);
uint8_t LargestSequence(uint8_t state);
uint8_t SmallestSequence(uint8_t state);



#endif








