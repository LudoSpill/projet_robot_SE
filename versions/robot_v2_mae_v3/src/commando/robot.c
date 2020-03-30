/* -*- mode: c; c-basic-offset: 4; indent-tabs-mode: nil -*- */  
#include <stdlib.h>
#include "robot.h"
#include "prose.h"

#define LEFT_MOTOR MD
#define RIGHT_MOTOR MA
#define LIGHT_SENSOR S1
#define FRONT_BUMPER S3
#define FLOOR_SENSOR S2


typedef struct {
	Motor *mD;	/* moteur droit */
	Motor *mG;	/* moteur gauche */
	LightSensor *lightSens;	/* capteur de luminosité */
	ContactSensor *contactSensFront;	/* capteur de contacts */
	ContactSensor *contactSensFloor;	/* capteur de contacts */
} Robot;

static Robot * robot;


/**
 * Start the Robot (initialize communication and open port)

 */
extern void Robot_start(){
	if (ProSE_Intox_init("127.0.0.1", 12345) == -1) {
		PProseError("Problème d'initialisation du simulateur Intox");
		exit(0);
	}
	Robot_new();
}


/**
 * Stop Robot (stop communication and close port)
 */
extern void Robot_stop(){
	Robot_free();
	ProSE_Intox_close();
}


/**
 * @briel initialize in memory the object Robot
 * 
 */
extern void Robot_new(){
	
	robot = (Robot *) malloc(sizeof(Robot));

	/* Initialisation des moteurs. */
	robot->mD = Motor_open(RIGHT_MOTOR);
	if (robot->mD == NULL) PProseError("Problème d'ouverture du moteur droit (port MA)");
	robot->mG = Motor_open(LEFT_MOTOR);
	if (robot->mG == NULL) PProseError("Problème d'ouverture du moteur gauche (port MB)");
	
	Robot_setWheelsVelocity(0,0);
	
	/* Initialisation des capteurs. */
	robot->lightSens = LightSensor_open(LIGHT_SENSOR);
	if (robot->lightSens == NULL) PProseError("Problème d'ouverture du détecteur de lumière (port S1)");

	robot->contactSensFront = ContactSensor_open(FRONT_BUMPER);
	if (robot->contactSensFront == NULL) PProseError("Problème d'ouverture du détecteur de contact");

}


/**
 *  @brief destruct the object Robot from memory 
 */
extern void Robot_free(){

	/* Fermeture des accès aux moteurs. */
	Motor_close(robot->mD);
	Motor_close(robot->mG);

	/* Fermeture des accès aux capteurs. */
	LightSensor_close(robot->lightSens);
	ContactSensor_close(robot->contactSensFront);


	free(robot);
}


/**
 * Robot_getRobotSpeed
 * 
 * @brief return the speed of the robot (positive average of the right's and left's current wheel power) 
 * @return speed of the robot (beetween 0 and 100)
 */
extern int Robot_getRobotSpeed(){
	Cmd speedDroite = Motor_getCmd(robot->mD);
	Cmd speedGauche = Motor_getCmd(robot->mG);

	int vitesse_robot = abs((speedGauche + speedDroite)/2);

	return vitesse_robot;

}

/**
 * Robot_getSensorState
 * 
 * @brief return the captor's states of the bumper and the luminosity
 * @return SensorState
 */
extern SensorState Robot_getSensorState(){
	SensorState etat_capteurs;

	etat_capteurs.luminosity = (float)LightSensor_getStatus(robot->lightSens);
	etat_capteurs.collision = ContactSensor_getStatus(robot->contactSensFront);

	return etat_capteurs;
}

/**
 * Robot_setWheelsVelocity
 * 
 * @brief set the power on the wheels of the robot
 * @param int mr : right's wheel power, value between -10O and 100
 * @param int ml : left's wheel power, value between -100 and 100
 */
extern void Robot_setWheelsVelocity(int mr, int ml){
	Motor_setCmd(robot->mD, mr);
	Motor_setCmd(robot->mG, ml);

}
