/*
 * asservissement.c
 *
 *  Created on: 13 mai 2010
 *      Author: mythril
 */

#include "asservissement.h"

// ----------------------------- //
// EXPOSITION AUX AUTRES MODULES //
// ----------------------------- //

// Compteur pulse codeur
volatile EncodeursValues encodeurs;

// Consigne orientation et distance robot
// Activation ou non de la deceleration en position finale
ConsignePolaire consignePolaire;

// ----------------------------------- //
// VARIABLES DE FONCTIONNEMENT INTERNE //
// ----------------------------------- //

signed long int ecartOrientationPrecedenteRobot;
signed long int ecartDistancePrecedenteRobot;
signed long int distanceDecelDistance, distanceDecelOrientation;
double vitesseCouranteDistance, vitesseCouranteOrientation = 0;

// Parametre de rŽgulation
struct PID_DATA pidDistance;
struct PID_DATA pidOrientation;

void initAsservissement(void) {
	// Initialisation des PID
	pid_Init(K_P_DISTANCE * SCALING_FACTOR, K_I_DISTANCE * SCALING_FACTOR , K_D_DISTANCE * SCALING_FACTOR , &pidDistance);
	pid_Init(K_P_ORIENTATION * SCALING_FACTOR, K_I_ORIENTATION * SCALING_FACTOR , K_D_ORIENTATION * SCALING_FACTOR , &pidOrientation);

	// LibŽration des moteurs
	FREE_DROIT;
	FREE_GAUCHE;
	generatePWM(0, 0);

	// Initialisation des variables interne
	consignePolaire.vitesseMoteurDistance = 100;
	consignePolaire.vitesseMoteurOrientation = 100;
	consignePolaire.consignePulseDistance = consignePolaire.consignePulseOrientation = 0;
	vitesseCouranteDistance = vitesseCouranteOrientation = 0;

	resetAsservissement();
}

void resetAsservissement(void) {
	//pid_Reset_Integrator(&pidDistance);
	//pid_Reset_Integrator(&pidOrientation);
}

/* rightPWM et leftPWM compris entre -100 et 100 % */
void generatePWM(signed char gauchePWM, signed char droitePWM) {
	if (droitePWM >= 0) AVANCE_DROIT; else RECULE_DROIT;
	if (gauchePWM >= 0) AVANCE_GAUCHE; else RECULE_GAUCHE;

	// Valeur absolu pour le PWM, le signe est dŽjˆ gŽrŽ au dessus.
	droitePWM = abs(droitePWM);
	gauchePWM = abs(gauchePWM);

	if (flags.flagLectureCodeur == 1) {
		OCR2A = fmin(gauchePWM, VITESSE_MOTEUR_MAX);
		OCR2B = fmin(droitePWM, VITESSE_MOTEUR_MAX);
	} else {
		OCR2B = fmin(droitePWM, VITESSE_MOTEUR_MAX);
		OCR2A = fmin(gauchePWM, VITESSE_MOTEUR_MAX);
	}
}

void asservissementPolaire(void) {
	// Calcul de la vitesse courante en fonction de la rampe d'acceleration et decelerration
	// GŽnŽration du profil trapŽzoidale de distance.
	distanceDecelDistance = mmToPulse((vitesseCouranteDistance * vitesseCouranteDistance) / (2 * RAMPE_DEC_DISTANCE));
	if (vitesseCouranteDistance > consignePolaire.vitesseMoteurDistance
		|| (abs(consignePolaire.consignePulseDistance) <= distanceDecelDistance && consignePolaire.activeFrein == ACTIVE_FREIN)) {

		vitesseCouranteDistance -= RAMPE_DEC_DISTANCE * TIME_ASSERV;

	} else if (vitesseCouranteDistance < consignePolaire.vitesseMoteurDistance) {

		vitesseCouranteDistance += RAMPE_ACC_DISTANCE * TIME_ASSERV;
	}

	// Calcul de la vitesse courante en fonction de la rampe d'acceleration et decelerration
	// GŽnŽration du profil trapŽzoidale d'orientation.
	distanceDecelOrientation = mmToPulse((vitesseCouranteOrientation * vitesseCouranteOrientation) / (2 * RAMPE_DEC_ORIENTATION));
	if (vitesseCouranteOrientation > consignePolaire.vitesseMoteurOrientation
		|| (abs(consignePolaire.consignePulseOrientation) <= distanceDecelOrientation && consignePolaire.activeFrein == ACTIVE_FREIN)) {

		vitesseCouranteOrientation -= RAMPE_DEC_ORIENTATION * TIME_ASSERV;

	} else if (vitesseCouranteOrientation < consignePolaire.vitesseMoteurOrientation) {

		vitesseCouranteOrientation += RAMPE_ACC_ORIENTATION * TIME_ASSERV;
	}

	// Interdire les valeurs nŽgatives sur les vitesse, et calcul des pulse thŽorique
	vitesseCouranteDistance = fmax(vitesseCouranteDistance, 0);
	vitesseCouranteOrientation = fmax(vitesseCouranteOrientation, 0);
	long int pulseForVitesseDistance = mmToPulse(vitesseCouranteDistance) * TIME_ASSERV;
	long int pulseForVitesseOrientation = mmToPulse(vitesseCouranteOrientation) * TIME_ASSERV;

	// Consigne de distance thŽorique en fonction de la vitesse et du temps ŽcoulŽ
	signed long int ecartTheoriqueDistance = pulseForVitesseDistance + abs(ecartDistancePrecedenteRobot);
	if (consignePolaire.consignePulseDistance < 0) {
		ecartTheoriqueDistance = -ecartTheoriqueDistance;
	}
	ecartDistancePrecedenteRobot = ecartTheoriqueDistance - encodeurs.nbEncocheDeltaDistance;

	signed long int ecartTheoriqueOrientation = pulseForVitesseOrientation + abs(ecartOrientationPrecedenteRobot);
	if (consignePolaire.consignePulseOrientation < 0) {
		ecartTheoriqueOrientation = -ecartTheoriqueOrientation;
	}
	ecartOrientationPrecedenteRobot = ecartTheoriqueOrientation - encodeurs.nbEncocheDeltaOrientation;

	// Calcul des pid
	signed int pidDistanceCalc = pid_Controller(ecartTheoriqueDistance, encodeurs.nbEncocheDeltaDistance, &pidDistance);
	signed int pidOrientationCalc = pid_Controller(ecartTheoriqueOrientation, encodeurs.nbEncocheDeltaOrientation, &pidOrientation);

	// Calcul des PWM
	signed int commandeDroit = pidDistanceCalc + pidOrientationCalc;
	signed int commandeGauche = pidDistanceCalc - pidOrientationCalc;

	// GŽnŽration de la PWM
	generatePWM(commandeGauche, commandeDroit);

	// Gestion des flags pour les arrets
	flags.flagTrajetAtteint = flags.flagApproche = 0;
	if (consignePolaire.activeFrein == ACTIVE_FREIN
			&& abs(consignePolaire.consignePulseDistance) < FENETRE_ARRET_DISTANCE
			&& abs(consignePolaire.consignePulseOrientation) < FENETRE_ARRET_ORIENTATION) {

		flags.flagTrajetAtteint = 1;

	} else if (consignePolaire.activeFrein == DESACTIVE_FREIN
			&& abs(consignePolaire.consignePulseDistance) < FENETRE_EN_APPROCHE_DISTANCE
			&& abs(consignePolaire.consignePulseOrientation) < FENETRE_EN_APPROCHE_ORIENTATION) {

		flags.flagApproche = 1;
	}
}
