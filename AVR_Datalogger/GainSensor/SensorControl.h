/** \file SensorControl.h
	 \brief Sensor control with Floating Point
*/

/**	 

	\ingroup avr_datalogger
 	\defgroup sensorControl Sensor Control with Floating Point
 	\code #include "SensorControl.h" \endcode

 	 
	 \par Description
	An avr-datalogger platform specific library which allows
	an interface with the sensor control hardware.
 	 
*/
//@{



#ifndef	_SENSOR_ROUTINES
#define	_SENSOR_ROUTINES


#include "mmculib/floatFunctions.h"

#define SENSOR_MACRO 0

#define SENSOR_ON (1)
#define SENSOR_OFF (0)

#define SENSOR_TEMP (0)
#define SENSOR_VOLTAGE (1)

#define SENSOR_COUNT (32)
#define GAIN_COUNT (5)

/** 22-bits resolution */
#define SENSOR_ENOB	(ADS1213_ENOB)
#define SENSOR_MAX 	(uint32_t)(2^(SENSOR_ENOB - 1))
#define SENSOR_SATURATION_LIMIT ((2^16) - 1)

#define SENSOR_REF_MULTIPLIER 100.0


extern uint32_t SensorType;
extern uint32_t SensorState;



void SensorTypeToggle(uint8_t channel);

/** Either SENSOR_ON or SENSOR_OFF */
uint8_t SensorGetState(uint8_t channel);
uint8_t SensorGetType(uint8_t channel);

/** Either SENSOR_TEMP or SENSOR_VOLTAGE */
void SensorSetType(uint8_t channel, uint8_t type);

/** Channel control routines */
void SensorOn(uint8_t channel);
void SensorOff(uint8_t channel);
void SensorToggle(uint8_t channel);

/** Channel Gain routines */
uint8_t SensorGetGain(uint8_t channel);
void SensorSetGain(uint8_t channel, uint8_t gain);

/** Condition the raw ADC value into a FP voltage */
float32_t SensorCondition(uint32_t data, uint8_t gainIndex);










#endif
