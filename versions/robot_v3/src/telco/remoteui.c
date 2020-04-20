#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <pthread.h>
#include <mqueue.h>
#include "remoteui.h"
#include "client.h"
#include "watchdog.h"

#define TIME_REFRESH_LOGS_SCREEN_SEC (1)

/**
 * DESCRIPTION DE LA MAE
 */

typedef enum {
	S_FORGET =0,
	S_INIT,
	S_CONNECTION,
	PS_TEST_CONNECTION,
	S_MAIN,
	S_LOGS,
	S_ERROR,
	S_DEATH,
	NB_ETATS
} Etat;

typedef enum {
	A_NOP = 0,
	A_DISPLAY_CONNECTION_SCREEN,
	A_DISPLAY_MAIN_SCREEN,
	A_DISPLAY_LOGS_SCREEN,
	A_DISPLAY_ERROR_SCREEN,
	A_TEST_CONNECTION,
	A_SET_DIR,
	A_TOGGLE_ES,
	A_SET_IP,
	A_QUIT,
	NB_ACTIONS
} Action;

typedef struct {
	Etat etatDestination;
	Action action;
} Transition;

static Transition myMae[NB_ETATS-1][NB_EVENTS]= {
	[S_INIT][E_GO_CONNECTION_SCREEN] = {S_CONNECTION,A_DISPLAY_CONNECTION_SCREEN},
	[S_CONNECTION][E_SET_IP] = {S_CONNECTION,A_SET_IP},
	[S_CONNECTION][E_VALIDATE] = {PS_TEST_CONNECTION,A_TEST_CONNECTION},
	[S_CONNECTION][E_QUIT] = {S_DEATH, A_QUIT},
	[PS_TEST_CONNECTION][E_CONNECTION_OK] = {S_MAIN,A_DISPLAY_MAIN_SCREEN},
	[PS_TEST_CONNECTION][E_CONNECTION_NOT_OK] = {S_ERROR,A_DISPLAY_ERROR_SCREEN},
	[S_ERROR][E_VALIDATE] = {S_CONNECTION,A_DISPLAY_CONNECTION_SCREEN},
	[S_MAIN][E_DISPLAY_LOGS_SCREEN] = {S_LOGS,A_DISPLAY_LOGS_SCREEN},
	[S_MAIN][E_SET_DIR] = {S_MAIN,A_SET_DIR},
	[S_MAIN][E_DISPLAY_MAIN_SCREEN] = {S_MAIN,A_DISPLAY_MAIN_SCREEN},
	[S_MAIN][E_TOGGLE_ES] = {S_MAIN,A_TOGGLE_ES},
	[S_MAIN][E_QUIT] = {S_DEATH, A_QUIT},
	[S_LOGS][E_DISPLAY_LOGS_SCREEN] = {S_LOGS,A_DISPLAY_LOGS_SCREEN},
	[S_LOGS][E_DISPLAY_MAIN_SCREEN] = {S_MAIN,A_DISPLAY_MAIN_SCREEN},
	[S_LOGS][E_TOGGLE_ES] = {S_MAIN,A_TOGGLE_ES},
	[S_LOGS][E_QUIT] = {S_DEATH, A_QUIT}

};

/**
 * FIN DE LA DESCRIPTION DE LA MAE
 */

//----------------------------FONCTIONS------------------------

static void * RemoteUI_run(void * arg);
static void RemoteUI_performAction(Action action);

static void handleConnectScreen();
static void handleErrorScreen();
static void handleMainScreen();
static void handleLogsScreen();
static void handleIpInput();
static void handleSetDir();
static void handleQuit();
static void getKeyboardInput();

static void start_handleLogScreen();
static void * showLogsScreen(void * unused);
static void waitEndOfDisplayLogScreen();
static void abortDisplayLogsScreen();
static void * getInputLogsScreen(void * unused);

static void setIp(char * myIp);
static void test_connexion();
static void Send_ES();

static void displayScreen();
static int captureChoice();

static PilotState ask4Log();
static void Send_client_request();

static void RemoteUI_mqInit();
static void RemoteUI_mqDone(void);
static void RemoteUI_mqReceive(MqMsg *this);
static void RemoteUI_mqSend(Event input);

//----------------------------FIN_FONCTIONS---------------------

typedef enum {OK =0, KO} Statut_Connexion;

typedef enum{
	MAIN_SCREEN = 0,
	INPUT_SCREEN,
	LOGS_SCREEN,
	CONNECT_SCREEN,
	IP_INPUT_SCREEN,
	ERROR_SCREEN,
	NB_SCREENS
} Screens;

//----------------------------VARIABLES-------------------------

static char input;
static Flag flag_timeout_refresh_logs_screen;
static int request = C_NULL;
PilotState logs;
static Statut_Connexion statut_connexion;
static pthread_t thread_remoteui_run;
static pthread_t thread_refresh_logs_screen;
static pthread_t thread_input_logs_screen;
static char err_msg[ERROR_MSG_MAX_LENGTH];
char myIp[IP_MAX_LENGTH];

static const char queueName[] = "/mailboxRemoteUI";
static mqd_t myBal;

//----------------------------FIN_VARIABLES-------------------------

// Tableau décrivant les réponses à une requete sur MAIN_SCREEN
static char * input_messages[NB_C_Request] = {	// Tableau dans le même ordre que C_Request
	"Vous avez demandé l'action : \nCommande non reconnue\n",
	"Vous avez demandé l'action : \navancer\n",
	"Vous avez demandé l'action : \nreculer\n",
	"Vous avez demandé l'action : \naller a droite\n",
	"Vous avez demandé l'action : \naller a gauche\n",
	"Vous avez demandé l'action : \nstopper\n",
	"Vous avez demandé l'action : \nafficher l'écran principal\n",
	"Vous avez demandé l'action : \nquitter\n",
	"Vous avez demandé l'action : \nafficher les logs du robot\n",
	"Vous avez demandé l'action : \narrêt d'urgence\n",
};

extern void RemoteUI_start(){

	printf("Bienvenue sur Robot V2\n");
	int err = pthread_create(&thread_remoteui_run,NULL,(void *)RemoteUI_run,NULL);
	if (err != 0){
		strcpy(err_msg, "Erreur de création du thread de run de remoteui...");
        handle_error(err, err_msg);
	}
	statut_connexion = KO;
	// Premier event : on affiche l'écran de connexion
	RemoteUI_mqSend(E_GO_CONNECTION_SCREEN);
}

extern void RemoteUI_stop(){

	printf("Merci d'avoir utilise Robot V2, a bientot !\n");
}

/**
 * initialize in memory RemoteUI
 * 
 */
extern void RemoteUI_new(){
	RemoteUI_mqInit();
}

/**
 * destruct the RemoteUI from memory 
 *
 */
extern void RemoteUI_free(){
	int err;
	err = pthread_join(thread_remoteui_run,NULL);
	/* handle error */
    if(err != 0){
        strcpy(err_msg, "Erreur de fermeture du thread de run de remoteui...");
        handle_error(errno, err_msg);
    }
	RemoteUI_mqDone();
}

static void * RemoteUI_run(void * arg){
	// Action de la transition initiale
	strncpy(myIp,"",IP_MAX_LENGTH);

	MqMsg message;
	Etat etatCourant = S_INIT;

	while(etatCourant != S_DEATH){
		RemoteUI_mqReceive(&message);
		trace("Event reçu : %d\n",message.event);
		if (myMae[etatCourant][message.event].etatDestination != S_FORGET){
			RemoteUI_performAction(myMae[etatCourant][message.event].action);
			etatCourant = myMae[etatCourant][message.event].etatDestination;
		}
	}
	trace("while fini\n");
	return 0;
}

static void RemoteUI_performAction(Action action){
	trace("Action réalisée : %d\n",action);
	switch (action)
	{
	case A_DISPLAY_CONNECTION_SCREEN:
		handleConnectScreen();
		break;

	case A_SET_IP:
		handleIpInput();
		break;

	case A_TEST_CONNECTION:
		test_connexion();
		break;

	case A_DISPLAY_ERROR_SCREEN:
		handleErrorScreen();
		break;
	
	case A_DISPLAY_MAIN_SCREEN:
		handleMainScreen();
		break;

	case A_SET_DIR:
		handleSetDir();
		break;

	case A_DISPLAY_LOGS_SCREEN:
		handleLogsScreen();
		break;

	case A_TOGGLE_ES:
		Send_ES(); // TODO
		break;

	case A_QUIT:
		handleQuit();
		RemoteUI_stop();
		break;

	default:
		trace("No action\n");
		break;
	}

}

/**
 * Fonctions de gestion des différents écrans
 */

static void handleConnectScreen(){
	displayScreen(CONNECT_SCREEN);
	request = captureChoice();
	if(request == C_QUIT){
		RemoteUI_mqSend(E_QUIT);
	}
	else{
		RemoteUI_mqSend(E_SET_IP);
	}
}

static void handleIpInput(){
	displayScreen(IP_INPUT_SCREEN);
	setIp(myIp);
	RemoteUI_mqSend(E_VALIDATE);
}

static void handleErrorScreen(){
	displayScreen(ERROR_SCREEN);
	getKeyboardInput();
	RemoteUI_mqSend(E_VALIDATE);
}

static void handleMainScreen(){
	
	//strcpy(&input,"");
	displayScreen(MAIN_SCREEN);
	request = captureChoice();
	displayScreen(INPUT_SCREEN);

	if(request >= C_NULL && request <= C_DISPLAY_MAIN)
	{
		RemoteUI_mqSend(E_SET_DIR);
	}
	else if (request == C_ES){
		RemoteUI_mqSend(E_TOGGLE_ES);
	}
	else if(request == C_DISPLAY_LOGS){
		RemoteUI_mqSend(E_DISPLAY_LOGS_SCREEN);
	}
	else if (request == C_QUIT){
		RemoteUI_mqSend(E_QUIT);
	}

}

static void handleSetDir(){
	if(request != C_NULL || request != C_DISPLAY_LOGS){
		Send_client_request();
	}

	RemoteUI_mqSend(E_DISPLAY_MAIN_SCREEN);
}

static void handleLogsScreen(){
	
	start_handleLogScreen();
	Watchdog * watchdog_refresh_logs_screen = Watchdog_construct(TIME_REFRESH_LOGS_SCREEN_SEC,abortDisplayLogsScreen);
	Watchdog_start(watchdog_refresh_logs_screen);
	waitEndOfDisplayLogScreen();
	Watchdog_cancel(watchdog_refresh_logs_screen);
	Watchdog_destroy(watchdog_refresh_logs_screen);
	pthread_cancel(thread_input_logs_screen);
}

static void start_handleLogScreen(){
	trace("start logs screen\n");
	flag_timeout_refresh_logs_screen = OFF;
	int err = pthread_create(&thread_refresh_logs_screen,NULL, &showLogsScreen,NULL);
    if(err != 0){
        strcpy(err_msg, "Erreur de création du thread de refresh Logs Screen...");
        handle_error(errno, err_msg);
    }
	err = pthread_create(&thread_input_logs_screen,NULL,&getInputLogsScreen,NULL);
	if(err != 0){
        strcpy(err_msg, "Erreur de création du thread de lecture de l'input dans l'état logs screen...");
        handle_error(errno, err_msg);
    }
}

static void * getInputLogsScreen(void * unused){
	request = captureChoice();
	trace("request : %d\n",request);
	return 0;
}

static void * showLogsScreen(void * unused){
	request = C_DISPLAY_LOGS; // On force la requête car si l'input est mauvais, captureChoice met request à C_NULL, ce qu'on ne veut pas
	ask4Log();
	displayScreen(LOGS_SCREEN);
	do{
		//request est modifié dans le thread thread_input_logs_screen
	} while(flag_timeout_refresh_logs_screen == OFF && request == C_DISPLAY_LOGS);

	if(request == C_DISPLAY_MAIN){
		RemoteUI_mqSend(E_DISPLAY_MAIN_SCREEN);
	}
	else if(request == C_ES){
		RemoteUI_mqSend(E_TOGGLE_ES);
	}
	else if(request == C_QUIT){
		RemoteUI_mqSend(E_QUIT);
	}
	else if(request != C_DISPLAY_LOGS){
		RemoteUI_mqSend(E_DISPLAY_LOGS_SCREEN);
	}

	return NULL;
}

static void waitEndOfDisplayLogScreen(){
	pthread_join(thread_refresh_logs_screen,NULL);
}

static void abortDisplayLogsScreen(){
	flag_timeout_refresh_logs_screen = ON;
	RemoteUI_mqSend(E_DISPLAY_LOGS_SCREEN);
}

//---------------------------------------------------

static void test_connexion(){
	statut_connexion = Client_start(myIp);;
	if(statut_connexion == OK){
		RemoteUI_mqSend(E_CONNECTION_OK);
	}
	else{
		RemoteUI_mqSend(E_CONNECTION_NOT_OK);
	}
}


static void Send_ES(){

}

static void handleQuit(){
	Send_client_request(request);
	if(statut_connexion == OK){
		Client_stop();
	}
}

static void displayScreen(int screen){
	
	switch (screen)
	{
	case (CONNECT_SCREEN):
		system("stty cooked echo");
		printf("\nVous pouvez effectuer les actions suivantes :\n");
		printf("!a : accéder à l'écran de connexion\n");
		printf("a : quitter\n");
		break;

	case (IP_INPUT_SCREEN):
		system("stty cooked echo");
		printf("\nEntrez l'IP du serveur au format 'xxx.xxx.xxx.xxx' :\n");
		break;

	case (ERROR_SCREEN):
		system("stty cooked echo");
		printf("\nImpossible de se connecter.\nAppuyez sur n'importe quelle touche pour être redirigé vers l'écran de connexion.\n");
		break;

	case (MAIN_SCREEN):
		system("stty cooked echo");
		printf("\nVous pouvez effectuer les actions suivantes :\n");
		printf("z : avancer\n");
		printf("s : reculer\n");
		printf("q : aller à gauche\n");
		printf("d : aller à droite\n");
		printf("  : stopper\n");
		printf("e : arrêt d'urgence\n");
		printf("r : afficher les logs du robot\n");
		printf("a : quitter\n");
		break;
	
	case (INPUT_SCREEN):
		system("stty cooked echo");
		printf("%s", input_messages[request]);
		break;

	case (LOGS_SCREEN):
		system("stty cooked echo");
		printf("\nVous pouvez effectuer les actions suivantes :\n");
		printf("t : retourner au menu principal\n");
		printf("e : arrêt d'urgence\n");
		printf("a : quitter\n\n");
		printf("Logs du robot :\n");
		printf("Vitesse : %d | Collision : %d | Lumière : %d\n", logs.speed, logs.collision, logs.luminosity);
		system("stty raw -echo");
		break;

	default:
		break;
	}
}

//---------------------------------------------------

static int captureChoice(){
	
	getKeyboardInput(); //modif input
	int retour = C_NULL;

	switch (input)
	{
	case ('z'):
		retour = C_FORWARD;
		break;
	case ('q'):
		retour = C_LEFT;
		break;
	case ('s'):
		retour = C_BACKWARD;
		break;
	case ('d'):
		retour = C_RIGHT;
		break;
	case ('e'):
		retour = C_ES;
		break;
	case ('r'):
		retour = C_DISPLAY_LOGS;
		break;
	case (' '):
		retour = C_STOP;
		break;
	case ('a'):
		retour = C_QUIT;
		break;
	case ('t'):
		retour = C_DISPLAY_MAIN;
		break;

	default:
		retour = C_NULL;
		break;
	}

	return retour;
}

//---------------------------------------------------

static void getKeyboardInput(){
	trace("J'attend un input\n");
	system("stty raw -echo");
	input = getchar();
	system("stty cooked echo");
	trace("J'ai reçu l'input : %c\n",input);
}

//---------------------------------------------------

static PilotState ask4Log(){
	Send_client_request();
	trace("Je demande les logs\n");
	logs = Client_readLog(); //étape bloquante
	trace("logs reçus\n");
	return logs;
}

//---------------------------------------------------

static void Send_client_request(){
	Client_sendMsg(request);	
}

static void setIp(char * ip){
	scanf("%s%*c",myIp);	// %*c permet de supprimer le \n du buffer
}

extern void handle_error(int err, char err_msg[]){

	if (err != 0){
		trace("%s\n",err_msg);
		trace("Message errno : %s\n",strerror(err));
		trace("Code errno : %d\n", err);
		exit(EXIT_FAILURE);
	}
	else
	{
		trace("%s\n",err_msg);
	}
 }

// --------------------- BOITE AUX LETTRES -----------------------------

static void RemoteUI_mqInit(){
    /* destruction de la BAL si toutefois préexistante */
	//mq_unlink(queueName);

    /* création et ouverture de la BAL */
	struct mq_attr mq_attributes;

	mq_attributes.mq_curmsgs = 0;					// Number of msg currently in queue
	mq_attributes.mq_flags  = 0;					// Number of msg currently in queue
	mq_attributes.mq_maxmsg = MQ_MAX_MESSAGES;	// Max number of message allowed in queue
	mq_attributes.mq_msgsize = sizeof(MqMsg);	// Message is the size of a Wrapper

	int flags =  O_RDWR | O_CREAT;

	/*
	 * Creation et ouverture de ma boite aux lettres
	 * Accessible en ecriture et en lecture pour l'utilisateur
	 * Accessible en lecture pour le groupe
	 * Accessible en lecture pour les autres
	*/
	myBal = mq_open(queueName, flags, 0644, &mq_attributes);
	/* handle error */
    if(errno != 0){
        strcpy(err_msg, "Erreur d'ouverture de la BAL...");
        handle_error(errno, err_msg);
    }
}

static void RemoteUI_mqDone(void){
	/* fermeture de la BAL */
	mq_close(myBal);
	/* handle error */
    if(errno != 0){
        strcpy(err_msg, "Erreur de fermeture de la BAL...");
        handle_error(errno, err_msg);
    }

	/* destruction de la BAL */
	mq_unlink(queueName);
	/* handle error */
    if(errno != 0){
        strcpy(err_msg, "Erreur de destruction de la BAL...");
        handle_error(errno, err_msg);
    }

}

static void RemoteUI_mqReceive(MqMsg *this){

    mq_receive(myBal,(char*)this,sizeof(MqMsg),0);
	if(errno != 0){
        strcpy(err_msg, "Erreur de lecture du message de la BAL...");
        handle_error(errno, err_msg);
    }
}

static void RemoteUI_mqSend(Event input){
     MqMsg msg_to_send = {
         .event = input
     };

     mq_send(myBal, msg_to_send.toString, sizeof(msg_to_send), 0);
     if(errno != 0){
        strcpy(err_msg, "Erreur d'envoi du message dans la BAL...");
        handle_error(errno, err_msg);
    }

 }
