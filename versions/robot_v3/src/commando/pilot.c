#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <mqueue.h>
#include <errno.h>
#include <string.h>
#include "pilot.h"
#include "robot.h"
#include "../telco/watchdog.h"

#define TIME_BUMP_CHECK (1)

// ---------------MAE------------------------------------------------

typedef enum {
    S_FORGET = 0, //ne se passe rien, sert à consommer l'event
    S_IDLE,
    S_RUNNING,
    PS_TEST_VEL,
    PS_TEST_BUMP,
    S_DEATH,
    NB_STATE
} Etat;

typedef enum{
    A_NOP = 0,
    A_SET_VELOCITY,
    A_TEST_VEl,
    A_ENTER_RUNNING_STATE,
    A_TEST_BUMP,
    NB_ACTION
} Action;

typedef struct 
{
    Etat etatDestination;
    Action action;
} Transition;

static Transition myMae[NB_STATE-1][NB_EVENT]=
{
    [S_IDLE][E_SET_VELOCITY] = {PS_TEST_VEL, A_TEST_VEl},
    [PS_TEST_VEL][E_VEL_NULLE] = {S_IDLE, A_SET_VELOCITY},
    [PS_TEST_VEL][E_VEL_NON_NULLE] = {S_RUNNING, A_ENTER_RUNNING_STATE},
    [S_RUNNING][E_SET_VELOCITY] = {PS_TEST_VEL, A_TEST_VEl},
    [S_RUNNING][E_CHECK_BUMP] = {PS_TEST_BUMP, A_TEST_BUMP},
    [PS_TEST_BUMP][E_BUMP_FALSE] = {S_RUNNING, A_ENTER_RUNNING_STATE},
    [PS_TEST_BUMP][E_BUMP_TRUE] = {S_IDLE, A_ENTER_RUNNING_STATE}
};
// ---------------FIN_MAE-------------------------------------------


// ---------------FONCTIONS----------------------------------------------------

static void * Pilot_run(void *unused);
static void Pilot_performAction(Action action);
static void Pilot_sendMvt(VelocityVector vel);
static void * Pilot_SendVel(void * vel);


static void handleSetVelocity();
static void handleTestVelocity();
static void handleTestBump();
static void handleRunningState();
static void start_handleRunningState();
static void waitEndOfRunningState();
static void * doNothing(void * unused);
static void abortRunningState();

static void Pilot_mqInit();
static void Pilot_mqDone(void);
static void Pilot_mqReceive(MqMsg *this);
static void Pilot_mqSend(Event input);

// ---------------FIN_FONCTIONS------------------------------------------------

// ---------------VARIABLES----------------------------------------------------

static char err_msg[ERROR_MSG_MAX_LENGTH];
static VelocityVector currentVel;
static const char queueName[] = "/mailboxPilot";
static mqd_t myBal;
static pthread_t threadRunPilot;
static Flag flag_test_bump;
static pthread_t thread_RunningState;
static Flag flag_input;


// ---------------FIN_VARIABLES------------------------------------------------

extern void Pilot_start(){
    Pilot_new();
   	Robot_start();
    int err = pthread_create(&threadRunPilot,NULL,(void *)Pilot_run,NULL);
	if (err != 0){
		strcpy(err_msg, "Erreur de création du thread de run de Pilot...");
        //handle_error(err, err_msg);
	}
}

extern void Pilot_stop(){
    Pilot_free();
    Robot_stop();
}

extern void Pilot_new(){
    Pilot_mqInit();
}

extern void Pilot_free(){
    int err;
	err = pthread_join(threadRunPilot,NULL);
	/* handle error */
    if(err != 0){
        strcpy(err_msg, "Erreur de fermeture du thread de run de Pilot...");
        ////handle_error(errno, err_msg);
    }
	Pilot_mqDone();
}

static void * Pilot_run(void * unused){
    // Action de la transition initiale
    currentVel.dir = STOP;
    currentVel.power = 0;
    
    MqMsg message;
    Etat etatCourant = S_IDLE;

    while (etatCourant != S_DEATH){
        Pilot_mqReceive(&message);
        flag_input = OFF;
        trace("Event lu dans la BAL : %d\n",message.event);
        if(myMae[etatCourant][message.event].etatDestination != S_FORGET){
            Pilot_performAction(myMae[etatCourant][message.event].action);
			etatCourant = myMae[etatCourant][message.event].etatDestination;
		}
    }
    return 0;
}

static void Pilot_performAction(Action action){
    switch (action)
    {
    case A_NOP:
        break;
    case A_SET_VELOCITY:
        handleSetVelocity();
        break;
    case A_TEST_VEl:
        handleTestVelocity();
        break;
    case A_TEST_BUMP:
        handleTestBump();
        break;
    case A_ENTER_RUNNING_STATE:
        handleRunningState();

    default:
        break;
    }
}

static void handleSetVelocity(){
    Pilot_sendMvt(currentVel);
}

static void handleTestVelocity(){
    if(currentVel.dir == STOP){
        Pilot_mqSend(E_VEL_NULLE);
    }
    else{
        Pilot_mqSend(E_VEL_NON_NULLE);
    }
}

static void handleRunningState(){
    Pilot_sendMvt(currentVel);
    start_handleRunningState();
    Watchdog * watchdog_check_bump = Watchdog_construct(TIME_BUMP_CHECK,abortRunningState);
    Watchdog_start(watchdog_check_bump);
    waitEndOfRunningState();
    trace("after Wait\n");
    Watchdog_cancel(watchdog_check_bump);
    Watchdog_destroy(watchdog_check_bump);
}

static void start_handleRunningState(){
    flag_test_bump = OFF;
    int err = pthread_create(&thread_RunningState,NULL, &doNothing,NULL);
    if(err != 0){
        strcpy(err_msg, "Erreur de création du thread de test du bump...");
        handle_error(errno, err_msg);
    }
}
static void * doNothing(void * unused){
    do{
        // Attente
    } while(flag_test_bump == OFF && flag_input == OFF);
    
    return NULL;
}

static void abortRunningState(){
    flag_test_bump = ON;
    Pilot_mqSend(E_CHECK_BUMP);
}

static void waitEndOfRunningState(){
    pthread_join(thread_RunningState,NULL);
}

static void handleTestBump(){
    if(Robot_getSensorState().collision == NO_BUMP){
        Pilot_mqSend(E_BUMP_FALSE);
    }
    else{
        Pilot_mqSend(E_BUMP_TRUE);
        currentVel.dir = STOP;
    }
}


extern void Pilot_setVelocity(VelocityVector vel){
    // pthread_t thread_set_vel;

    // int err = pthread_create(&thread_set_vel,NULL, &Pilot_SendVel,vel);
    // if(err != 0){
    //     strcpy(err_msg, "Erreur de création du thread de test du bump...");
    //     handle_error(errno, err_msg);
    // }
    Pilot_mqSend(E_SET_VELOCITY);
    currentVel = vel;
    flag_input = ON;
}

// static void * Pilot_SendVel(void * vel){
//     Pilot_mqSend(E_SET_VELOCITY);
//     currentVel = vel;
//     return NULL;
// }

static void Pilot_sendMvt(VelocityVector vel){
    int vel_r =0 , vel_l = 0;
    switch (vel.dir)
    {
    case (LEFT):
        vel_r = -vel.power;
        vel_l = vel.power;
        break;
    case (RIGHT):
        vel_r = vel.power;
        vel_l = -vel.power;
        break;
    case (FORWARD):
        vel_r = vel.power;
        vel_l = vel.power;
        break;
    case (BACKWARD):
        vel_r = -vel.power;
        vel_l = -vel.power;
        break;
    case (STOP):
        break;
    
    default:
        break;
    }

    Robot_setWheelsVelocity(vel_r,vel_l);
}

/**
 * getState
 * 
 * @brief description 
 * @return PilotState
 */
/* extern PilotState Pilot_getState(){
    
} */

/**
 * CHECK
 * 
 * @brief description 
 */
extern void Pilot_CHECK(){

}



// --------------------- BOITE AUX LETTRES -----------------------------

static void Pilot_mqInit(){
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
        //handle_error(errno, err_msg);
    }
}

static void Pilot_mqDone(void){
	/* fermeture de la BAL */
	mq_close(myBal);
	/* handle error */
    if(errno != 0){
        strcpy(err_msg, "Erreur de fermeture de la BAL...");
        //handle_error(errno, err_msg);
    }

	/* destruction de la BAL */
	mq_unlink(queueName);
	/* handle error */
    if(errno != 0){
        strcpy(err_msg, "Erreur de destruction de la BAL...");
        //handle_error(errno, err_msg);
    }

}

static void Pilot_mqReceive(MqMsg *this){

    mq_receive(myBal,(char*)this,sizeof(MqMsg),0);
	if(errno != 0){
        strcpy(err_msg, "Erreur de lecture du message de la BAL...");
        //handle_error(errno, err_msg);
    }
}

static void Pilot_mqSend(Event input){
     MqMsg msg_to_send = {
         .event = input
     };

     mq_send(myBal, msg_to_send.toString, sizeof(msg_to_send), 0);
     if(errno != 0){
        strcpy(err_msg, "Erreur d'envoi du message dans la BAL...");
        //handle_error(errno, err_msg);
    }

 }