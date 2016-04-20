/** \file GainControl.h
	 \brief Gain control
*/

/**	 

	\ingroup avr_datalogger
 	\defgroup gainControl Gain Control
 	\code #include "GainControl.h" \endcode

 	 
	 \par Description
	An avr-datalogger platform specific library which allows
	an interface with the gain control hardware.
 	 
*/
//@{

#ifndef	_GAIN_CONTROL
#define	_GAIN_CONTROL

#define GS_MAX7300_ADDRESS   (0x98)
#define GS_SENSOR_PORT       (0x4C)
#define GS_GAIN_PORT         (0x52)

#define GS_SENSOR_MASK       (0x1F)
#define GS_GAIN_MASK         (0x3F)


#define GS_GAIN_A    (4)
#define GS_GAIN_B    (2)
#define GS_GAIN_C    (3)
#define GS_GAIN_D    (5)
#define GS_GAIN_E    (0)
#define GS_GAIN_F    (1)


#define GS_ReadRegister(reg)  MAX7300_ReadRegister(GS_MAX7300_ADDRESS, reg)
#define GS_SetRegister(reg, data)  MAX7300_SetRegister(GS_MAX7300_ADDRESS, reg, data)


/** Switch combinations which produce the given gain */
#define GS_GAIN_9P6	( (1 << GS_GAIN_A) | (1 << GS_GAIN_B) | (1 << GS_GAIN_C) | (1 << GS_GAIN_F))
#define GS_GAIN_2P5	( (1 << GS_GAIN_B) | (1 << GS_GAIN_C) | (1 << GS_GAIN_F) )
#define GS_GAIN_0P4	( (1 << GS_GAIN_D) | (1 << GS_GAIN_F))
#define GS_GAIN_0P09	( (1 << GS_GAIN_C) | (1 << GS_GAIN_E) | (1 << GS_GAIN_F))
#define GS_GAIN_0P04	( (1 << GS_GAIN_D) | (1 << GS_GAIN_E))

enum {
   GAIN9P6X = 0, GAIN2P5X, GAIN0P4X,
   GAIN0P09X, GAIN0P04
} GainsM;

/** Gains in FP */
#define GAIN_9P6FP	9.615
#define GAIN_2P5FP	2.472
#define GAIN_0P4FP	0.4167
#define GAIN_0P09FP	0.0866
#define GAIN_0P04FP	0.0417

//Gains with 3 implied decimal places
#define GAIN_9P6	9615
#define GAIN_2P5	2472
#define GAIN_0P41	417
#define GAIN_0P09	87
#define GAIN_0P04	42


/** Gain offsets */
#define GAIN_9P6O		450e3
#define GAIN_2P5O		110e3
#define GAIN_0P4O		14e3
#define GAIN_0P09O	2500
#define GAIN_0P04O	0

extern const uint8_t GS_GAIN[];
extern const float SENSOR_GAIN[];
extern const uint32_t GAIN_OFFSETS[];

/** GS_Init:
* Initialises the MAX7300 responsible for controlling Gain 
*/
void GS_Init(void);

/** Changes the mutliplexer channel 
   \param channel Limited to 1 to 32 inclusive */
void GS_Channel(uint8_t channel);

/** Changes the gain
   \param gain Switch combinations as defined by GS_GAIN_XPX */
void GS_GainSel(uint8_t gain);





#endif

