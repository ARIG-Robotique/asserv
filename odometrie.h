/*
 * odometrie.h
 *
 *  Created on: 31 déc. 2010
 *      Author: mythril
 */

#ifndef ODOMETRIE_H_
#define ODOMETRIE_H_

#include "define.h"
#include "asservissement.h"
#include "conversion.h"
#include "rs232.h"

#include <math.h>

#define LINEAIRE 		1
#define CIRCULAIRE 		2

#define TYPE_AVANT 		0
#define TYPE_ARRIERE 	1

// Structure de position en Pulse
typedef struct {
	long int x;
	long int y;
	long int theta;
} RobotPosition;

typedef struct {
	RobotPosition pos;
	char frein : 1;
} RobotConsigne;

//  y (2000)
//  |
//  |
//  |
//  |
//  |
//  |---------------------------------- x (3000)
// 0,0
// Theta = 0 dans le sens de Y

extern RobotConsigne consigneTable;
extern RobotPosition positionCourrante;

void initPosition(long double x, long double y, long double theta);
void calcConsigneAsserv(void);
void calcPosition(void);

#endif /* ODOMETRIE_H_ */
