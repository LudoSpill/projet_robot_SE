/**
 * Programme principal de hello robot.
 */
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <signal.h>
#include <unistd.h>

#include "prose.h"
#include "robot/robot.h"

/* Prototype de la fonction exemple de manipulation du robot. */
void hello_robot(Robot * robot);


/* Variable de sortie du programme. */
static int keepGoing = 1;

/**
 * Fonction permettant de gérer le CTRL+C
 */
static void intHandler(int dummy)
{
    keepGoing = 0;
}

/**
 * Fonction principale.
 */
int main (int argc, char *argv[])
{
	Robot * robot;

	printf("Programme exemple Hello Robot v0.8 : Ctrl+C pour quitter\n");
	fflush(stdout);

#ifdef INTOX
	/* Initialisation pour l'utilisation du simulateur Intox. */
	if (ProSE_Intox_init("127.0.0.1", 12345) == -1) {
		PProseError("Problème d'initialisation du simulateur Intox");
		return EXIT_FAILURE;
	}
#endif

	/* Interception d'un Ctrl+C pour arrêter le programme proprement. */
	signal(SIGINT, intHandler);

	/* Construction d'un robot. */
	robot = robot_C();

	/* Procédure exemple de manipulation du robot. */
	hello_robot(robot);

	/* Arrêt du robot. */
	robot_stop(robot);

	/* Destruction du robot. */
	robot_D(robot);

#ifdef INTOX
	/* Arrêt de l'utilisation du simulateur Intox. */
	ProSE_Intox_close();
#endif

	return EXIT_SUCCESS;
}

/**
 * Procédure exemple de manipulation du robot.
 *
 * Applique des consignes aux robots avec une temporisation 
 * entre les consignes successives.
 *
 * \param robot le robot à manipuler.
 */
void hello_robot(Robot * robot)
{
	/* Variables impliquées dans la temporisation. */
	int clocks_per_msec = CLOCKS_PER_SEC / 1000;
	int tps_attente;
	volatile clock_t c0, c1;

	/* DT */
	int tempo = 2;		/* en secondes */
	Cmd roueD = -100;		/* consigne sur la roue droite. */
	Cmd roueG = 100;		/* consigne sur la roue gauche. */

	while(keepGoing)
	{
		/* Pour la gestion de la temporisation. */
		c0 = clock();

		/* Consignes au robot. */
		robot_consignes(robot, roueD, roueG);

		/* Gestion de la temporisation. */
		c1 = clock();

		tps_attente = (tempo * CLOCKS_PER_SEC - c1 + c0) / clocks_per_msec;
		printf("attente = %d \n", tps_attente);

		if (tps_attente > 0) {
			usleep(tps_attente * 1000);
		} else {
			printf("\ntemps de traitement > %d sec ! \n\n", tempo);
		}
		fflush(stdout);
	}
}

