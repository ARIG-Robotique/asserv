/*
 * define.h
 *
 *  Created on: 5 mai 2010
 *      Author: mythril
 */

/*
 * PD0 (0)  : RX
 * PD1 (1)  : TX
 *
 * PD2 (2)  : SIGN Moteur DROIT
 * PD3 (3)  : PWM Moteur DROIT (OC2B)
 *
 * PD4 (4)  : Codeur DROIT (T0)
 * PD5 (5)  : Codeur GAUCHE (T1)
 *
 * PD7 (7)  : Alarme moteur DROIT
 * PB0 (8)	: Alarme moteur GAUCHE
 *
 * PB1 (9)  : BREAK Moteur GAUCHE
 * PB3 (11) : PWM Moteur GAUCHE (OC2A)
 * PB4 (12) : BREAK Moteur DROIT
 * PB5 (13) : SIGN Moteur GAUCHE
 *
 * AN0 (A0) : Courant moteur DROIT
 * AN1 (A1) : Courant Moteur GAUCHE
 */

#ifndef DEFINE_H_
#define DEFINE_H_

#define FOSC 				16000000 	// Clock Speed

// Liste des commandes
#define TEST				't'
#define STOP				's'
#define AVANCE				'a'
#define RECULE				'r'
#define TOURNE_GAUCHE		'g'
#define	TOURNE_DROIT		'd'
#define VITESSE_PARAM		'v'

typedef struct {
	unsigned char flagAsserv : 1;
	unsigned char flagLectureCodeur : 1;
	//unsigned char flagAdc : 1;
	//unsigned char flagAdcCompleted : 1;
	unsigned char flagDebug : 1;
	unsigned char flagApproche : 1;
	unsigned char flagTrajetAtteint : 1;
} FlagsGestion;

extern volatile FlagsGestion flags;

#endif /* DEFINE_H_ */
