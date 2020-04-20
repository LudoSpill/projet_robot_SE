#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include "remoteui.h"
#include "client.h"

static void run();
static void display();
static void captureChoice();
static void quit();

static void askCmd();
static void ask4Log();
static void askClearLog();
static void Send_mvt_request();

typedef enum{
	D_MENU = 0,
	D_INPUT,
	NB_D_Request
} D_Request;

static char *input_messages[9] = {	// Tableau dans le même ordre que C_Request
	"Vous avez demandé l'action : \nCommande non reconnue\n",
	"Vous avez demandé l'action : \navancer\n",
	"Vous avez demandé l'action : \nreculer\n",
	"Vous avez demandé l'action :\naller a droite\n",
	"Vous avez demandé l'action :\naller a gauche\n",
	"Vous avez demandé l'action : \nstopper\n",
	"Vous avez demandé l'action : quitter\n",
	"Vous avez demandé l'action : \nafficher l'état du robot\n",
	"Vous avez demandé l'action : \neffacer les logs\n"
	
};

static char input;
static Flag flag;
static int request = C_NULL;

/**
 * Start RemoteUI and waits for the user's input until the user ask to quit
 *
 */
extern void RemoteUI_start(){

	printf("Bienvenue sur Robot V2\n");
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
	printf("Merci d'avoir utilise Robot V2, a bientot !\n");

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


static void display(int type){
	
	switch (type)
	{
	case (D_MENU):
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
		break;
	
	case (D_INPUT):
		printf("%s", input_messages[request]);
		break;

	default:
		break;
	}
	
}

static void captureChoice(){
	
	system("stty raw -echo");
	input = getchar();
	system("stty cooked echo");

	switch (input)
	{
	case ('z'):
		request = C_FORWARD;
		break;
	case ('q'):
		request = C_LEFT;
		break;
	case ('s'):
		request = C_BACKWARD;
		break;
	case ('d'):
		request = C_RIGHT;
		break;
	case ('e'):
		request = C_CLEAR_LOGS;
		break;
	case ('r'):
		request = C_DISPLAY_LOGS;
		break;
	case (' '):
		request = C_STOP;
		break;
	case ('a'):
		request = C_QUIT;
		flag = ON;
		break;

	default:
		request = C_NULL;
		break;
	}

	askCmd();

	if (flag == OFF){
		display(D_INPUT);
		display(D_MENU);
	}
	else
	{
		display(D_INPUT);
		RemoteUI_stop();
		Client_stop();
	}

}

static void run(){
	display(D_MENU);
	system("stty raw -echo");
	while(flag == OFF) {
		captureChoice();
	}
	
	system("stty cooked echo");
}

static void askCmd(){
	if(request >= C_FORWARD && request <= C_QUIT)
	{
		Send_mvt_request();
	}
	else if (request == C_DISPLAY_LOGS)
	{
		ask4Log();
	}
	else if (request == C_CLEAR_LOGS)
	{
		askClearLog();
	}

}

static void ask4Log(){
	request = C_DISPLAY_LOGS;
	Send_mvt_request();
	Robot_Logs logs = Client_readLog();		//étape bloquante
	printf("Vitesse : %d | Collision : %d | Lumière : %d\n", logs.robot_speed, logs.bump, logs.luminosity);

}

static void askClearLog(){
	for(int i=0;i<15;i++){
		printf("\n");
	}
}

static void Send_mvt_request(){
	Client_sendMsg(request);	
}

extern void handle_error(int err, char err_msg[]){

	if (err != 0){
		printf("%s\n",err_msg);
		printf("Message errno : %s\n",strerror(err));
		printf("Code errno : %d\n", err);
		exit(EXIT_FAILURE);
	}
	else
	{
		printf("%s\n",err_msg);
	}
 }
