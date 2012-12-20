/*
 * asservissement.h
 *
 *  Created on: 13 mai 2010
 *      Author: mythril
 */

#ifndef ASSERVISSEMENT_H_
#define ASSERVISSEMENT_H_

#include <avr/io.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include "define.h"
#include "pid.h"
#include "rs232.h"
#include "conversion.h"

#define CYCLE_ASSERV				78			// Asserv 128 us * 78 = 9,984 ms
#define TIME_ASSERV					0.009984	// En s

#define RESOLUTION_CODEUR			512
#define PPR 						512 		// pulse per rotation : 512 (d = 40mm)

#define PERIMETRE_ROUE				124			// Perimetre de la roue en mm (PI * 40)
#define RAYON 						19			// Rayon des roues en mm (~2cm)
#define ENTRAXE						161			// Entraxe des roue codeuse en mm
#define ENTRAXE_PULSE				651.084		// Entraxe en pulse

#define RAMPE_ACC_DISTANCE			500.0 // en mm/s2
#define RAMPE_DEC_DISTANCE			100.0 // en mm/s2

#define RAMPE_ACC_ORIENTATION		500.0 // en mm/s2
#define RAMPE_DEC_ORIENTATION		100.0 // en mm/s2

#define FENETRE_ARRET_DISTANCE			4  // +- 4 pulse pour l'arret -> 1mm
#define FENETRE_ARRET_ORIENTATION		11 // +- 11,36 pulse pour l'arret -> 1°

#define FENETRE_EN_APPROCHE_DISTANCE	80 // +- 80 pulse pour la reception de la nouvelle position -> 2 cm
#define FENETRE_EN_APPROCHE_ORIENTATION 57 // +- 56,8 pulse pour la reception de la nouvelle position -> 5°

// Definition des bits de lecture codeurs
#define CHA_GAUCHE			(PIND >> PORTD5) & 1
#define CHB_GAUCHE			(PIND >> PORTD4) & 1

#define CHA_DROIT			(PINC >> PORTC2) & 1
#define CHB_DROIT			(PINC >> PORTC3) & 1

// Vitesse maximale des moteurs pour la génération de la PWM
// Unité en % (PWM)
#define VITESSE_MOTEUR_MAX 			204 // 80%

// Definition des I/O de commande du moteur droit
#define STOP_DROIT 			PORTB |= (1 << PORTB4)
#define FREE_DROIT			PORTB &= ~(1 << PORTB4)
#define AVANCE_DROIT 		PORTD &= ~(1 << PORTD2)
#define RECULE_DROIT 		PORTD |= (1 << PORTD2)

#define SENS_DROIT			((PORTD >> PORTD2) & 1) // Sens de rotation

// Definition des I/O de commande du moteur gauche
#define STOP_GAUCHE 		PORTB |= (1 << PORTB1)
#define FREE_GAUCHE			PORTB &= ~(1 << PORTB1)
#define AVANCE_GAUCHE 		PORTB &= ~(1 << PORTB5)
#define RECULE_GAUCHE 		PORTB |= (1 << PORTB5)

#define SENS_GAUCHE			((PORTB >> PORTB5) & 1) // Sens de rotation

// Constante d'activation / desactivation du frein en position finale
#define DESACTIVE_FREIN		0
#define ACTIVE_FREIN		1

// Parametres PID
#define K_P_DISTANCE   		1.50
#define K_I_DISTANCE   		0.00
#define K_D_DISTANCE   		0.00

#define K_P_ORIENTATION		1.50
#define K_I_ORIENTATION		0.00
#define K_D_ORIENTATION		0.00

// Structures pour la gestion des encodeurs
typedef struct {
	signed int nbEncochesDroitRealA;
	signed int nbEncochesGaucheRealA;
	signed int nbEncochesDroitRealB;
	signed int nbEncochesGaucheRealB;
	signed int nbEncochesDroit4Calc;
	signed int nbEncochesGauche4Calc;

	signed long int nbEncocheDeltaDistance;
	signed long int nbEncocheDeltaOrientation;
} EncodeursValues;

// Structure pour la consigne moteur (polaire)
typedef struct {
	signed long int consignePulseDistance;
	signed long int consignePulseOrientation;
	unsigned int vitesseMoteurDistance;
	unsigned int vitesseMoteurOrientation;
	unsigned char activeFrein : 1;
} ConsignePolaire;

// Compteur pulse codeur
extern volatile EncodeursValues encodeurs;

// Consigne orientation et distance robot
// Activation ou non de la deceleration en position finale
extern ConsignePolaire consignePolaire;

void initAsservissement(void);
void resetAsservissement(void);
void asservissementPolaire(void);

void generatePWM(signed char gauchePWM, signed char droitePWM);

#endif /* ASSERVISSEMENT_H_ */
