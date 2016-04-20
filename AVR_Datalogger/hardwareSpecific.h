/*
 * hardwareSpecific.h
 *
 *  Created on: 1/04/2016
 *      Author: AdrianDesk
 */

#ifndef HARDWARESPECIFIC_H_
#define HARDWARESPECIFIC_H_


#include <avr/io.h>


#define ADS1213_CLK        (2e6)
#define ADS1213_CS_DDR     (DDRA)
#define ADS1213_CS_PIN     (PA0)
#define ADS1213_CS_PORT    (PORTA)



#endif /* HARDWARESPECIFIC_H_ */
