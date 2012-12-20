/*
 * odometrie.c
 *
 *  Created on: 31 déc. 2010
 *      Author: mythril
 */

#include "odometrie.h"

RobotConsigne consigneTable;
RobotPosition positionCourrante;

void initPosition(long double x, long double y, long double theta) {
	positionCourrante.x = x;
	consigneTable.pos.x = x;
	positionCourrante.y = y;
	consigneTable.pos.y = y;
	positionCourrante.theta = theta;
	consigneTable.pos.theta = theta;
	consigneTable.frein = ACTIVE_FREIN;
}

// Calcul de la distance en pulse et de l'orientation pour l'asservissement
void calcConsigneAsserv(void) {
	/*if (flags.flagTrajetAtteint == 0) {
		// Calcul en fonction de l'odométrie
		double dX = consigneTable.pos.x - positionCourrante.x;
		double dY = consigneTable.pos.y - positionCourrante.y;

		long int alpha = radToPulse(atan2(pulseToRad(dY), pulseToRad(dX)));

		consignePolaire.consignePulseDistance = sqrt(pow(dX, 2) + pow(dY, 2));
		consignePolaire.consignePulseOrientation = alpha - positionCourrante.theta;
	} else {*/
		consignePolaire.consignePulseDistance -= encodeurs.nbEncocheDeltaDistance;
		consignePolaire.consignePulseOrientation -= encodeurs.nbEncocheDeltaOrientation;
	//}
	consignePolaire.activeFrein = consigneTable.frein;
}

// Calcul du nouveau X, Y, Theta
void calcPosition(void) {
	// Approximation linéaire
	positionCourrante.theta += encodeurs.nbEncocheDeltaOrientation;
	long double thetaRad = pulseToRad(positionCourrante.theta);
	double dX = encodeurs.nbEncocheDeltaDistance * cos(thetaRad);
	double dY = encodeurs.nbEncocheDeltaDistance * sin(thetaRad);
	positionCourrante.x += dX;
	positionCourrante.y += dY;

	// Approximation circulaire (pas testé)
	/*long double r = encodeurs.nbEncocheDeltaDistance * mmToPulse(ENTRAXE) / (encodeurs.nbEncocheDeltaOrientation * 2);
	long double arcAngle = 2 * encodeurs.nbEncocheDeltaOrientation / (mmToPulse(ENTRAXE));
	positionCourrante.x += r * (-sin(positionCourrante.theta) + sin(positionCourrante.theta + arcAngle));
	positionCourrante.y += r * (cos(positionCourrante.theta) - cos(positionCourrante.theta + arcAngle));
	positionCourrante.theta += arcAngle;*/
}
