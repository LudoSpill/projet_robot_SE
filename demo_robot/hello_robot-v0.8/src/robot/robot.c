/*
 * \file mon_robot.c
 *
 * \see mon_robot.h
 *
 * \author Matthias Brun
 */
#include <stdlib.h>
#include <stdio.h>

#include "robot.h"
#include "prose.h"

#define ROBOT_CMD_STOP (0)

/**
 * Constructeur de robot.
 *
 * \see robot.h
 */
Robot * robot_C()
{
	Robot * this;
	
	this = (Robot *) malloc(sizeof(Robot));

	/* Initialisation des moteurs. */
	this->mD = Motor_open(MD);
	if (this->mD == NULL) PProseError("Problème d'ouverture du moteur droit (port MD)");

	this->mG = Motor_open(MA);
	if (this->mG == NULL) PProseError("Problème d'ouverture du moteur gauche (port MA)");

	return this;

}

/**
 * Destructeur de mon robot.
 *
 * \see robot.h
 */
void robot_D(Robot * robot)
{
	/* Fermeture des accès aux moteurs. */
	Motor_close(robot->mD);
	Motor_close(robot->mG);

	free(robot);
}


/**
 * Consignes sur les roues de mon robot.
 *
 * \see robot.h
 */
void robot_consignes(Robot * robot, int roueD, int roueG)
{
	/* Appliquer les consignes aux moteurs. */
	if (Motor_setCmd(robot->mD, roueD) == -1) {
		PProseError("Problème de commande du moteur droit");
	}
	if (Motor_setCmd(robot->mG, roueG) == -1) {
		PProseError("Problème de commande du moteur gauche");
	}
}

/**
 * Arrêt de mon robot (vitesses moteurs à zéro).
 *
 * \see robot.h
 */
void robot_stop(Robot * robot)
{
	/* Arrêt des moteurs. */
	robot_consignes(robot, ROBOT_CMD_STOP, ROBOT_CMD_STOP);
}

