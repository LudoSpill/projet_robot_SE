/* -*- mode: c; c-basic-offset: 4; indent-tabs-mode: nil -*- */  
// a completer
   


#include <stdlib.h>
#include <stdio.h>
#include "adminui.h"
#include "pilot.h"
#include "robot.h"

static void run();
static void display();
static void captureChoice();
static void quit();
static void askMvt(Direction dir);

static void ask4Log();
static void askClearLog();

typedef enum {ON, OFF} Flag;


static char input;
static Flag flag;

/**
 * Start AdminUI and waits for the user's input until the user ask to quit
 *
 */
extern void AdminUI_start(){

	printf("Bienvenue sur Robot V1\n");
	Pilot_start();	
	flag = OFF;
	run();

}

/**
 * Stop AdminUI
 *
 */
extern void AdminUI_stop(){

	quit();
	Pilot_stop();
	printf("Merci d'avoir utilise Robot V1, a bientot !\n");

}

static void quit(){

	system("stty cooked echo");
}


/**
 * initialize in memory AdminUI
 * 
 */
extern void AdminUI_new(){

}

/**
 * destruct the AdminUI from memory 
 *
 */
extern void AdminUI_free(){

}


static void display(){

	system("stty cooked echo");
	printf("Vous pouvez effectuer les actions suivantes :\n");
	printf("z : avancer\n");
	printf("s : reculer\n");
	printf("q : aller à gauche\n");
	printf("d : aller à droite\n");
	printf("  : stopper\n");
	printf("e : effacer les logs\n");
	printf("r : afficher l'état du robot\n");
	printf("a : quitter\n");
	
}

static void captureChoice(){
	
	system("stty raw -echo");
	input = getchar();
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
		ask4Log();
		break;
	case (' '):
		printf("Vous avez demandé l'action : \nstopper\n");
		askMvt(STOP);
		break;
	case ('a'):
		printf("Vous avez demandé l'action : quitter\n");
		askMvt(STOP);
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



static void run(){
	display();
	system("stty raw -echo");
	while(flag == OFF) {
		captureChoice();
	}
	
	system("stty cooked echo");

}


static void askMvt(Direction dir){
	VelocityVector vel;
	switch (dir)
	{
	case FORWARD:
		vel.power = 80;
		break;
	case BACKWARD: vel.power = 60;
		break;
	case RIGHT: vel.power = 50;
		break;
	case LEFT: vel.power = 50;
		break;
	case STOP: vel.power = 0;
		break;
	
	default:
		break;
	}
	vel.dir = dir;
	Pilot_setVelocity(vel);
}

static void ask4Log(){
	printf("Vitesse : %d | Collision : %d | Lumière : %f\n", Robot_getRobotSpeed(), Robot_getSensorState().collision, Robot_getSensorState().luminosity);
}

static void askClearLog(){
	for(int i=0;i<15;i++){
		printf("\n");
	}
}
 