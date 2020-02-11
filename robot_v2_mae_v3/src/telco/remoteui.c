/* -*- mode: c; c-basic-offset: 4; indent-tabs-mode: nil -*- */  
// a completer
   


#include <stdlib.h>
#include <stdio.h>
#include "remoteui.h"
#include "client.h"
//#include "../commando/pilot.h"
//#include "../commando/robot.h"

/* static void setIP(ip ip);
 */static void run();
static void display();
static void captureChoice();
static void quit();
static void askMvt(Direction dir);
static void ask4Log();
static void askClearLog();
static void Send_request();

typedef enum {ON, OFF} Flag;

typedef enum{
	C_NULL = 0,
	C_FORWARD,
	C_BACKWARD,
	C_RIGHT,
	C_LEFT,
	C_STOP,
	C_DISPLAY_LOGS,
	C_QUIT
} C_Request;


static char input;
static Flag flag;
static C_Request request = C_NULL;

/**
 * Start RemoteUI and waits for the user's input until the user ask to quit
 *
 */
extern void RemoteUI_start(){

	printf("Bienvenue sur Robot V1\n");
	Client_start();
	flag = OFF;
	run();
}

/**
 * Stop RemoteUI
 *
 */
extern void RemoteUI_stop(){

	quit();
	RemoteUI_free();
	printf("Merci d'avoir utilise Robot V1, a bientot !\n");

}

static void quit(){

	system("stty cooked echo");
}


/**
 * initialize in memory RemoteUI
 * 
 */
extern void RemoteUI_new(){

}

/**
 * destruct the RemoteUI from memory 
 *
 */
extern void RemoteUI_free(){

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
		RemoteUI_stop();
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
	switch (dir)
	{
	case FORWARD:
		request = C_FORWARD;
		break;
	case BACKWARD: 
		request = C_BACKWARD;
		break;
	case RIGHT: 
		request = C_RIGHT;
		break;
	case LEFT: 
		request = C_LEFT;
		break;
	case STOP: 
		request = C_STOP;
		break;
	
	default:
		request = C_NULL;
		break;
	}
	Send_request();
}

static void ask4Log(){
	//printf("Vitesse : %d | Collision : %d | Lumière : %f\n", Robot_getRobotSpeed(), Robot_getSensorState().collision, Robot_getSensorState().luminosity);
	request = C_DISPLAY_LOGS;
	Send_request();
}

static void askClearLog(){
	for(int i=0;i<15;i++){
		printf("\n");
	}
}

static void Send_request(){
	Client_start();
	Client_sendMsg(request);
	Client_stop();
}

 