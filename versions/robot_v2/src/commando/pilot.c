#include <stdlib.h>
#include <stdio.h>
#include "pilot.h"
#include "robot.h"

typedef enum {
    S_FORGET = 0, //ne se passe rien, sert à consommer l'event
    S_IDLE,
    S_RUNNING,
    PS_TEST_VEL,
    PS_TEST_BUMP,
    S_DEATH,
    NB_STATE
} State;

typedef enum{
    A_NOP = 0,
    A_SET_VELOCITY,
    A_TEST_VEl,
    A_TEST_BUMP,
    A_STOP
} TransitionAction;

typedef enum {
    E_SET_VELOCITY,
    E_CHECK_BUMP,
    E_VEL_NON_NULLE,
    E_VEL_NULLE,
    E_BUMP_TRUE,
    E_BUMP_FALSE,
    E_STOP,
    NB_EVENT
} Event;

typedef struct 
{
    State destinationState;
    TransitionAction action;
} Transition;

static Transition stateMachine[NB_STATE][NB_EVENT]=
{
    [S_IDLE][E_SET_VELOCITY]={PS_TEST_VEL,A_TEST_VEl},
    //[S_IDLE][E_STOP]={S_DEATH,A_STOP},
    [PS_TEST_VEL][E_VEL_NULLE]={S_IDLE,A_SET_VELOCITY},
    [PS_TEST_VEL][E_VEL_NON_NULLE]={S_RUNNING,A_SET_VELOCITY},
    [S_RUNNING][E_SET_VELOCITY]={PS_TEST_VEL,A_TEST_VEl},
    [S_RUNNING][E_CHECK_BUMP]={PS_TEST_BUMP,A_TEST_BUMP},
    //[S_RUNNING][E_STOP]={S_DEATH,A_STOP},
    [PS_TEST_BUMP][E_BUMP_FALSE]={S_RUNNING,A_NOP},
    [PS_TEST_BUMP][E_BUMP_TRUE]={S_IDLE,A_SET_VELOCITY}
};

State currentState = S_DEATH;

static void Pilot_run(Event event, VelocityVector vel);
static void Pilot_performAction(TransitionAction action, VelocityVector vel);
static void Pilot_sendMvt(VelocityVector vel);
static void Pilot_evalVelocity(VelocityVector vel);


static void Pilot_run(Event event, VelocityVector vel){
    TransitionAction anAction;
    State nextState;

    anAction = stateMachine[currentState][event].action;
    nextState = stateMachine[currentState][event].destinationState;

    if(nextState != S_FORGET){    
        currentState = nextState;
        Pilot_performAction(anAction, vel);
    }
}

static void Pilot_performAction(TransitionAction action, VelocityVector vel){
    switch (action)
    {
    case A_NOP:
        break;
    case A_SET_VELOCITY:
        Pilot_sendMvt(vel);
    case A_TEST_VEl:
        Pilot_evalVelocity(vel);
        break;
    case A_TEST_BUMP:
        // TODO
        break;
    case A_STOP:
        Pilot_sendMvt(vel);
        break;
    default:
        break;
    }
}

static void Pilot_evalVelocity(VelocityVector vel){
    if(vel.power == 0){
        Pilot_run(E_VEL_NULLE,vel);
    }
    else{
        Pilot_run(E_VEL_NON_NULLE,vel);
    }
}

/**
 * Start Pilot
 */
extern void Pilot_start(){
    Pilot_new();
   	Robot_start();
}

/**
 * Stop Pilot
 */
extern void Pilot_stop(){
    Pilot_free();
    Robot_stop();
}

/**
 * initialize in memory the object Pilot
 */
extern void Pilot_new(){
    currentState = S_IDLE;
}

/**
 * destruct the object Pilot from memory 
 */
extern void Pilot_free(){
    currentState = S_DEATH;
}

/**
 * setVelocity
 * 
 * @brief description 
 * @param vel 
 */
extern void Pilot_setVelocity(VelocityVector vel){
    Pilot_run(E_SET_VELOCITY, vel);
}

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
