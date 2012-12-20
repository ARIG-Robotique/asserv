/*
 * rs232.c
 *
 *  Created on: 5 mai 2010
 *      Author: mythril
 */

#include "rs232.h"

// Variable de reception RS232
volatile char cDataReceive; 	// Contient le dernier caract�re re�u
volatile char bData;			// Indique une donn�e pr�sente
char cMessage[30];		// Tableau contenant le message envoy� par la RS232

// Initialisation de la liaison serie
void initRS232(void) {
	/*Set baud rate */
	UBRR0 = MYUBRR;

	// Activation de la reception et emission.
	// La reception se produit par interruption (sous programme dans le main)
	UCSR0B = (1 << RXCIE0) | (1 << RXEN0) | (1 << TXEN0);

	// Trame :
	//	mode asynchronous
	//  8 bits
	//  1 bit de stop
	//  Pas de parit�
	UCSR0C = (0 << UMSEL01) | (0 << UMSEL00)
			| (0 << UPM01) | (0 << UPM00)
			| (0 << USBS0)
			| (0 << UCSZ02) | (1 << UCSZ01) | (1 << UCSZ00);
}

// Envoie une donn�e sur 8bits (char)
void envoiChar(char data) {
	// Attente que le buffer d'emission soit vide
	while (!(UCSR0A & (1 << UDRE0)));

	// Ecriture de l'information
	UDR0 = data;
}

// Emission de donn�e sur le port s�rie
void envoiMessage(char * data){
	char iCmpt;
	int taille;

	taille = strlen(data);

    for(iCmpt = 0 ; iCmpt < taille ; iCmpt++) {
		envoiChar(data[iCmpt]);
    }
}
