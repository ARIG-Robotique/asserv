/*
 * main.c
 *
 *  Created on: 28 Décembre 2010
 *      Author: Gregory DEPUILLE
 *      Author: Guillaume LEMAITRE
 */

#include <avr/io.h>
#include <avr/interrupt.h>

#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "define.h"
#include "asservissement.h"
#include "odometrie.h"
#include "conversion.h"
#include "rs232.h"

volatile FlagsGestion flags;
volatile unsigned char cpt;


void portInitialise(void);
void timerInitialise(void);
void pinIntInitialise(void);

ISR(WDT_vect) {
	flags.flagDebug = 1;
}

// Interruption lors de la réception d'un caractère sur la liaison série
ISR(USART_RX_vect) {
	unsigned char error;
	// If frame error or parity error or data overRun
	if ((UCSR0A & (1 << FE0)) || (UCSR0A & (1 << UPE0))
			|| (UCSR0A & (1 << DOR0))) {
		error = UDR0;
	} else { // no error
		cDataReceive = UDR0;
		bData = 1;
	}
}

/* interruption sur debordement timer 2 : toutes les 128us */
ISR(TIMER2_OVF_vect) {
	cpt++;
	if (cpt >= CYCLE_ASSERV) { // cf define.h
		cpt = 0;
		flags.flagAsserv = 1;
	}

	TIFR2 |= (1 << TOV2);
}

/* Interruption sur changement d'etat de la broche CHA GAUCHE */
// Le codeur gauche tourne à l'envers par rapport au droit.
ISR(PCINT2_vect) {
	if (CHA_GAUCHE) {
		// Front montant de CHA Gauche
		if (flags.flagLectureCodeur == 0) {
			encodeurs.nbEncochesGaucheRealA += (CHB_GAUCHE) ? 1 : -1;
		} else {
			encodeurs.nbEncochesGaucheRealB += (CHB_GAUCHE) ? 1 : -1;
		}
	} /*else {
		// Front descendant de CHA Gauche
		if (flags.flagLectureCodeur == 0) {
			encodeurs.nbEncochesGaucheRealA += (!CHB_GAUCHE) ? 1 : -1;
		} else {
			encodeurs.nbEncochesGaucheRealB += (!CHB_GAUCHE) ? 1 : -1;
		}
	}*/
}

/* Interruption sur changement d'etat de la broche CHA DROIT */
ISR(PCINT1_vect) {
	if (CHA_DROIT) {
		// Front montant de CHA Droit
		if (flags.flagLectureCodeur == 0) {
			encodeurs.nbEncochesDroitRealA += (CHB_DROIT) ? -1 : 1;
		} else {
			encodeurs.nbEncochesDroitRealB += (CHB_DROIT) ? -1 : 1;
		}
	} /*else {
		// Front descendant de CHA Droit
		if (flags.flagLectureCodeur == 0) {
			encodeurs.nbEncochesDroitRealA += (!CHB_DROIT) ? -1 : 1;
		} else {
			encodeurs.nbEncochesDroitRealB += (!CHB_DROIT) ? -1 : 1;
		}
	}*/
}

/*
 * Configuration des I/O
 *
 * PD0 (0)  : RX
 * PD1 (1)  : TX
 *
 * PD2 (2)  : SIGN Moteur DROIT
 * PD3 (3)  : PWM Moteur DROIT (OC2B)
 *
 * PD4 (4)  : CH B Codeur GAUCHE
 * PD5 (5)  : CH A Codeur GAUCHE
 *
 * PD7 (7)  : Alarme moteur DROIT
 * PB0 (8)	: Alarme moteur GAUCHE
 *
 * PB1 (9)  : BREAK Moteur GAUCHE
 * PB3 (11) : PWM Moteur GAUCHE (OC2A)
 * PB4 (12) : BREAK Moteur DROIT
 * PB5 (13) : SIGN Moteur GAUCHE
 *
 * PC2 (25) : CH A Codeur DROIT
 * PC3 (26) : CH B Codeur DROIT
 *
 * AN0 (A0) : Courant moteur DROIT
 * AN1 (A1) : Courant Moteur GAUCHE
 */
void portInitialise(void) {
	/* Configuration des ports : '1' pour sortie */

	// Port B : PB1, PB2, PB3, PB4 et PB5 en sortie, PB0 en entrée
	DDRB = (1 << PORTB5) | (1 << PORTB4) | (1 << PORTB3) | (1 << PORTB2) | (1 << PORTB1) | (0 << PORTB0);
	PORTB = 0x00; /* PORTB a 0 */

	// Port C en sortie (Contient l'ADC) : AN0 et AN1 en entrée, PC2 et PC3 en entrée
	DDRC = 0xF0;
	PORTC = 0x00; /* PORTC a 0 */

	// Port D : PD1, PD2, PD3 - PD4, PD5 et PD7 en entree
	// PD4 et 5 sont utilisé pour compter les pulses codeurs (cf page 111)
	DDRD = (0 << PORTD7) | (0 << PORTD5) | (0 << PORTD4) | (1 << PORTD3) | (1 << PORTD2) | (1 << PORTD1) | (0 << PORTD0);
	PORTD = 0x00; /* PORTD a 0 */
}

/* Le timer 2 est utilise pour generer le PWM */
void timerInitialise(void) {
	// PRTWI : Désactivation horloge I2C
	// PRTIM2 : Activation horloge Timer/Counter2 module
	// PRTIM1 : Desactivation horloge Timer/Counter1 module
	// PRTIM0 : Desactivation horloge Timer/Counter0 module
	// PRSPI : Désactivation horloge SPI
	// PRUSART0 : Activation horloge RS232
	// PRADC : Desactivation horloge ADC
	PRR = (1 << PRTWI) | (0 << PRTIM2) | (1 << PRTIM1) | (1 << PRTIM0) | (1 << PRSPI) | (0 << PRUSART0) | (1 << PRADC);

	// Active le mode de synchronisation pour le Timer/Counter
	GTCCR |= (1 << TSM);

	// Configuration TIMER 2
	TCCR2A = (1 << COM2A1) | (0 << COM2A0) | (1 << COM2B1) | (0 << COM2B0) // non-inverting mode
			| (1 << WGM21) | (1 << WGM20); // fast mode - TOP = 0xFF
	TCCR2B = (0 << WGM22) | (0 << CS22) | (1 << CS21) | (0 << CS20); // prescaler 8 : Fpwm = 7.8kHz
	OCR2A = 0;
	OCR2B = 0;
	TCNT2 = 0;
	TIFR2 = (1 << TOV2); // enables interrupt
	TIMSK2 = (1 << TOIE2);
}

/*
 * Initialisation des broches d'interruption
 * PCINT 20 : Comptage pour le codeur DROIT
 * PCINT 21 : Comptage pour le codeur GAUCHE
 */
void pinIntInitialise(void) {
	// Activation des broches d'interruption
	PCICR = (1 << PCIE2) | (1 << PCIE1) | (0 << PCIE0);

	// Interruption sur la broche PCINT21 (CHA GAUCHE)
	// Interruption sur la broche PCINT20 (CHB GAUCHE)
	PCMSK2 = (0 << PCINT23) | (0 << PCINT22) | (1 << PCINT21) | (0 << PCINT20)
			| (0 << PCINT19) | (0 << PCINT18) | (0 << PCINT17) | (0 << PCINT16);

	// Interruption sur la broche PCINT10 (CHA DROIT)
	// Interruption sur la broche PCINT11 (CHB DROIT)
	PCMSK1 = (0 << PCINT14) | (0 << PCINT13) | (0 << PCINT12) | (0 << PCINT11)
			| (1 << PCINT10) | (0 << PCINT9) | (0 << PCINT8);
}

// MAIN //
int main(void) {

	UCSR0B = 0; // the bootloader enables usart - we disable it to use PD0 and PD1

	portInitialise();
	initRS232();
	timerInitialise();
	pinIntInitialise();

	/* Activation de toute les intérruptions */
	SREG = (1 << SREG_I);

	initPosition(mmToPulse(100), mmToPulse(100), 0);
	initAsservissement();

	int cpt = 0;
	int idxTrajet = 0;

	while (1) {
		if (bData != 0) {
			bData = 0;

			switch(cDataReceive) {
				case 's' :
					switch(idxTrajet) {
						case 0:
							//consigneTable.pos.x = mmToPulse(1000);
							//consigneTable.pos.y = mmToPulse(100);
							consignePolaire.consignePulseDistance = mmToPulse(1000);
							consignePolaire.consignePulseOrientation = 0;
							break;
						case 1:
							//consigneTable.pos.x = mmToPulse(1000);
							//consigneTable.pos.y = mmToPulse(500);
							consignePolaire.consignePulseDistance = mmToPulse(1000);
							consignePolaire.consignePulseOrientation = degToPulse(90);
							break;
/*
						case 2:
							consigneTable.pos.x = mmToPulse(1500);
							consigneTable.pos.y = mmToPulse(250);
							break;

						case 3:
							consigneTable.pos.x = mmToPulse(200);
							consigneTable.pos.y = mmToPulse(300);
							break;

						case 4:
							consigneTable.pos.x = mmToPulse(100);
							consigneTable.pos.y = mmToPulse(100);
							break;
							*/
					}
					consigneTable.frein = ACTIVE_FREIN;

					idxTrajet++;
					flags.flagTrajetAtteint = flags.flagApproche = 0;
					break;

				case 'r' :
					initPosition(mmToPulse(100), mmToPulse(100), 0);
					initAsservissement();
					idxTrajet = 0;
					break;

				case VITESSE_PARAM :
					while(bData == 0);
					bData = 0;
					consignePolaire.vitesseMoteurDistance = consignePolaire.vitesseMoteurOrientation = cDataReceive;
					break;

				case 'i' :
					consignePolaire.vitesseMoteurDistance = consignePolaire.vitesseMoteurOrientation = 100;
					break;

				case 'o' :
					consignePolaire.vitesseMoteurDistance -= 100;
					consignePolaire.vitesseMoteurDistance = fmax(consignePolaire.vitesseMoteurDistance, 0);
					consignePolaire.vitesseMoteurOrientation = consignePolaire.vitesseMoteurDistance;
					break;

				case 'p' :
					consignePolaire.vitesseMoteurDistance += 100;
					consignePolaire.vitesseMoteurOrientation = consignePolaire.vitesseMoteurDistance;
					break;
			}

		}

		if (flags.flagAsserv == 1) {
			flags.flagAsserv = 0;
			if (flags.flagLectureCodeur == 1) {
				flags.flagLectureCodeur = 0;
				encodeurs.nbEncochesDroit4Calc = encodeurs.nbEncochesDroitRealB;
				encodeurs.nbEncochesGauche4Calc = encodeurs.nbEncochesGaucheRealB;
				encodeurs.nbEncochesDroitRealB = encodeurs.nbEncochesGaucheRealB = 0;
			} else {
				flags.flagLectureCodeur = 1;
				encodeurs.nbEncochesGauche4Calc = encodeurs.nbEncochesGaucheRealA;
				encodeurs.nbEncochesDroit4Calc = encodeurs.nbEncochesDroitRealA;
				encodeurs.nbEncochesDroitRealA = encodeurs.nbEncochesGaucheRealA = 0;
			}

			// Calcul des retours consigne
			encodeurs.nbEncocheDeltaDistance = (encodeurs.nbEncochesDroit4Calc + encodeurs.nbEncochesGauche4Calc) / 2;
			encodeurs.nbEncocheDeltaOrientation = encodeurs.nbEncochesDroit4Calc - encodeurs.nbEncochesGauche4Calc;

			calcPosition(); // Mise a jour de l'odométrie
			calcConsigneAsserv(); // Calcul des consignes
			asservissementPolaire(); // Asservissement en fonction des consignes

			// DEBUG
			cpt++;
			if (cpt > 10) {
				cpt = 0;

				sprintf(cMessage, "%d;%d;%d", (int) pulseToMm(positionCourrante.x), (int) pulseToMm(positionCourrante.y), (int) pulseToDeg(positionCourrante.theta));
				envoiMessage(cMessage);

				sprintf(cMessage, ";%d;%d", (int) pulseToMm(consigneTable.pos.x), (int) pulseToMm(consigneTable.pos.y));
				envoiMessage(cMessage);

				sprintf(cMessage, ";%d;%d", (int) pulseToMm(consignePolaire.consignePulseDistance), (int) pulseToDeg(consignePolaire.consignePulseOrientation));
				envoiMessage(cMessage);

				sprintf(cMessage, ";%d;%d;%d", consignePolaire.activeFrein, flags.flagApproche, flags.flagTrajetAtteint);
				envoiMessage(cMessage);

				sprintf(cMessage, ";%d;%d", idxTrajet, flags.flagDebug);
				envoiMessage(cMessage);

				sprintf(cMessage, "\r\n");
				envoiMessage(cMessage);
			}
		}
	}

	return 0;
}
