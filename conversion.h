/*
 * conversion.h
 *
 *  Created on: 5 mai 2010
 *      Author: mythril
 */

#ifndef CONVERSION_H_
#define CONVERSION_H_

#include <math.h>

#define COUNT_PER_MM				4.044	 			// Nombre d'impulsion codeur pour 1mm
#define COUNT_PER_DEG				11.36 				// 5.68 * 2 Nombre de count pour réalisé 1°
#define COUNT_PER_RAD				325.542 * 2			// Nombre de count pour réalisé 1 radian

long int mmToPulse(long int);
long int pulseToMm(long int);
long int degToPulse(long double);
long double pulseToDeg(long int);
long double pulseToRad(long int);
long int radToPulse(long double);

#endif /* CONVERSION_H_ */
