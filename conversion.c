/*
 * conversion.c
 *
 *  Created on: 5 mai 2010
 *      Author: mythril
 */

#include "conversion.h"

// Conversion des mm en pulse
long int mmToPulse(long int val) {
	return val * COUNT_PER_MM;
}

// Conversion des pulse en mm
long int pulseToMm(long int val) {
	return val / COUNT_PER_MM;
}

// Conversion des ¡ en pulse
long int degToPulse(long double val) {
	return val * COUNT_PER_DEG;
}

// Conversion des pulse en ¡
long double pulseToDeg(long int val) {
	return val / COUNT_PER_DEG;
}

long double pulseToRad(long int val) {
	//return val / COUNT_PER_RAD;
	return pulseToDeg(val * M_PI / 180);
}

long int radToPulse(long double val) {
	//return val * COUNT_PER_RAD;
	return degToPulse(val * 180 / M_PI);
}
