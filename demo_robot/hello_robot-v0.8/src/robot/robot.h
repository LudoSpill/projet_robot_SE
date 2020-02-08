/**
 * \file robot.h
 *
 * \author Matthias Brun.
 */
#include "prose.h"

#ifndef ROBOT_H_
#define ROBOT_H_

/**
 * \struct Robot robot.h
 *
 * Un robot muni de deux moteurs. 
 */
typedef struct {
	Motor * mD;	/**< Le moteur droit. */
	Motor * mG;	/**< Le moteur gauche. */
} Robot;


/**
 * Constructeur de robot.
 */
Robot * robot_C();

/**
 * Destructeur de robot.
 *
 * \param robot le robot à détruire.
 */
void robot_D(Robot * robot);

/**
 * Consignes sur les roues du robot.
 *
 * \param robot le robot auquel s'adressent les consignes.
 * \param roueD consigne pour la roue droite (moteur droit).
 * \param roueG consigne pour la roue gauche (moteur gauche).
 */
void robot_consignes(Robot * robot, int roueD, int roueG);

/**
 * Arrêt du robot (vitesses moteurs à zéro).
 *
 * \param robot le robot à arrêter.
 */
void robot_stop(Robot * robot);

/**
 * Affiche les informations sur les capteurs du robot.
 *
 * \param robot le robot concerné.
 */
void robot_affiche_informations(Robot * robot);

#endif /* ROBOT_H_ */

