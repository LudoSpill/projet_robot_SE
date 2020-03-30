#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include "server.h"
#include "pilot.h"
#include "robot.h"


#define MAX_PENDING_CONNECTIONS (5)

#define MAX_LOGS_LENGTH (128)

static void Server_give_order();
static void Server_send_mvt(Direction direction);
static void Server_send_robot_state();

static int Tab_Dir_Pow[NB_DIR] = {VEL_POWER_LEFT,
     VEL_POWER_RIGHT, 
     VEL_POWER_FORWARD, 
     VEL_POWER_BACKWARD, 
     VEL_POWER_STOP
}; //même ordre que l'enum Direction


int socket_ecoute;
int socket_communication_ext;
struct sockaddr_in adresse_serveur;
int message;
static volatile Flag flag_stop;

char err_msg[ERROR_MSG_MAX_LENGTH];


extern void Server_start(){
    flag_stop = OFF;
    Pilot_start();
    socket_ecoute = socket(AF_INET, SOCK_STREAM, 0);

    if(socket_ecoute == -1){
        /* handle error */
        if(errno != 0){
            strcpy(err_msg, "Erreur : le socket écoute n'a pas pu être créé");
            handle_error(errno, err_msg);
        }
    }
    
    adresse_serveur.sin_family = AF_INET;
    adresse_serveur.sin_port = htons(PORT_DU_SERVEUR);
    adresse_serveur.sin_addr.s_addr = htonl(INADDR_ANY);

    /**
     * Portion de code permettant de lier le serveur au même port instantanément après avoir fermé le socket
     */
    int yes=1;
    if (setsockopt(socket_ecoute, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) == -1) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }
    /**
     * Fin portion
     */

    if(bind(socket_ecoute, (struct sockaddr *) &adresse_serveur, sizeof(adresse_serveur)) == -1){
        /* handle error */
        if(errno != 0){
            strcpy(err_msg, "Erreur : le socket écoute ne s'est pas lié au serveur");
            handle_error(errno, err_msg);
        }
    }
    else
    {
        printf("Socket écoute lié au serveur...\n");
    }
    
    if(listen(socket_ecoute, MAX_PENDING_CONNECTIONS) == -1){
        /* handle error */
        if(errno != 0){
            strcpy(err_msg, "Erreur : le socket serveur n'a pas pu écouter les connexions entrantes");
            handle_error(errno, err_msg);
        }
    }
    else
    {
        printf("Socket écoute prêt à écouter...\n");
    }
        
    socket_communication_ext = accept(socket_ecoute, NULL, 0);
    /* handle error */
    if(errno != 0){
        strcpy(err_msg, "Erreur de lecture du message par le client...");
        handle_error(errno, err_msg);
    }
}

extern void Server_stop(){
    close(socket_ecoute);
}

extern void Server_readMsg(){
    
    // if(read(socket_communication_ext, &message, sizeof(message)) == -1){   /* ATTENTION : fonction bloquante */
    //     printf("Erreur de lecture du message par le client.\n");
    //     exit(EXIT_FAILURE);
    // }////
    // printf("Message reçu par le serveur : %s\n",message);

    while(flag_stop == OFF){
        /* Acceptation de la connexion */
        //socket_communication_ext = accept(socket_ecoute, NULL, 0);
        //if (fork() == 0){     /* fork() : duplication de la tâche, on assigne 0 à la tâche créée */
            if(read(socket_communication_ext, &message, sizeof(message)) == -1){   /* ATTENTION : fonction bloquante */
                /* handle error */
                if(errno != 0){
                    printf("Message envoyé par le client : %d\n",message);
                    strcpy(err_msg, "Erreur de lecture du message envoyé par le client");
                    handle_error(errno, err_msg);
                }
                exit(EXIT_FAILURE);
            }
            printf("Message reçu par le serveur : %d\n",message);
            Server_give_order();
        //}
    }
}

static void Server_give_order(){
    switch (message)
    {
    case C_NULL:
        break;
    case C_FORWARD:
        Server_send_mvt(FORWARD);
        break;
    case C_BACKWARD:
        Server_send_mvt(BACKWARD);
        break;
    case C_RIGHT:
        Server_send_mvt(RIGHT);
        break;
    case C_LEFT:
        Server_send_mvt(LEFT);
        break;
    case C_STOP:
        Server_send_mvt(STOP);
        break;
    case C_DISPLAY_LOGS:
        Server_send_robot_state();
        break;
    case C_QUIT:
        Server_send_mvt(STOP);
        Pilot_stop();
        flag_stop = ON;
        break;
    
    default:
        break;
    }
}

static void Server_send_mvt(Direction direction){
    VelocityVector vel;
    vel.dir = direction;
    vel.power = Tab_Dir_Pow[vel.dir];
    Pilot_setVelocity(vel);
}

static void Server_send_robot_state(){

    char logs[MAX_LOGS_LENGTH];
    sprintf(logs,"%d %d %d",Robot_getRobotSpeed(),Robot_getSensorState().collision,(int)Robot_getSensorState().luminosity);
    if(write(socket_communication_ext, &logs, sizeof(logs)) == -1){
        /* handle error */
        if(errno != 0){
        strcpy(err_msg, "Erreur d'envoi de l'etat du robot...");
        handle_error(errno, err_msg);
        }
    }; 
}
