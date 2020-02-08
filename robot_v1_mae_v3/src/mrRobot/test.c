/* #include <stdlib.h>
#include <stdio.h>
#include "adminui.h"
#include "pilot.h"
#include "robot.h"

static void captureChoice(){
	
	system("stty raw -echo");
	char input = getchar();
	system("stty cooked echo");

	switch (input)
	{
	case ('z'):
		printf("Vous avez demandé l'action : \navancer\n");
        askMvt(FORWARD);
		break;
	case ('q'):
		printf("Vous avez demandé l'action : \naller a gauche\n");
        askMvt(LEFT);
		break;
	case ('s'):
		printf("Vous avez demandé l'action : \nreculer\n");
        askMvt(BACKWARD);
		break;
	case ('d'):
		printf("Vous avez demandé l'action :\naller a droite\n");
        askMvt(RIGHT);
		break;
	case ('e'):
		printf("Vous avez demandé l'action : \neffacer les logs\n");
		askClearLog();
		break;
	case ('r'):
		printf("Vous avez demandé l'action : \nafficher l'état du robot\n");
		printf("Vitesse : %d | Collision : %d | Lumière : %f\n", Robot_getRobotSpeed(), Robot_getSensorState().collision, Robot_getSensorState().luminosity);
		break;
	case (' '):
		printf("Vous avez demandé l'action : \nstopper\n");
		vel.power = 0;
		Pilot_setVelocity(vel);
		break;
	case ('a'):
		printf("Vous avez demandé l'action : quitter\n");
		vel.power = 0;
		Pilot_setVelocity(vel);
		flag = ON;
		break;

	default:
		printf("Vous avez demandé l'action : \nCommande non reconnue\n");
		break;
	}
	if (flag == OFF){
			display();
		}
}


static void askMvt(VelocityVector vel){
    VelocityVector vel;
	vel.dir = dir;

    switch (dir)
    {
    case FORWARD:
        vel.power = 80;
		Pilot_sendMvt(vel);
        break;
    case BACKWARD:
        vel.power = 60;
        Pilot_sendMvt(vel);
        break;
    case RIGHT:
        vel.power = 50;
        Pilot_sendMvt(vel);
    case LEFT:
        vel.power = 50;
        Pilot_sendMvt(vel);
    case STOP:
        vel.power = 0;
        Pilot_sendMvt(vel);
    default:
        break;
    }
} */