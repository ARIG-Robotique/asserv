/*
 * rs232.h
 *
 *  Created on: 5 mai 2010
 *      Author: mythril
 */

/*
 * RX (Digital PIN 0) : Reception RS232 TTL
 * TX (Digital PIN 1) : Transmission RS232 TTL
 */

#ifndef RS232_H_
#define RS232_H_

#include <avr/io.h>
#include <string.h>

#include "define.h"

// Configuration de la liaison s�rie
#define BAUD 	9600
#define MYUBRR  103	//FOSC/16/BAUD-1

// Prototype
void initRS232(void);
void envoiChar(char);
void envoiMessage(char *);

// Variable de reception RS232
extern volatile char cDataReceive; 	// Contient le dernier caract�re re�u
extern volatile char bData;			// Indique une donn�e pr�sente
extern char cMessage[30];			// Tableau contenant le message envoy� par la RS232

#endif /* RS232_H_ */
