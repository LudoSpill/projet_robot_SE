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

#define MAX_PENDING_CONNECTIONS (5)
#define VEL_POWER_FORWARD (80)
#define VEL_POWER_BACKWARD (60)
#define VEL_POWER_RIGHT (50)
#define VEL_POWER_LEFT (50)
#define VEL_POWER_STOP (0)


static void Server_give_order();
static void Server_send_mvt(Direction direction);

static int Tab_Dir_Pow[NB_DIR] = {50, 50, 80, 60, 0}; //même ordre que l'enum Direction

typedef enum{
    OFF, ON
}Flag_stop;

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

int socket_ecoute;
int socket_communication_ext;
struct sockaddr_in adresse_serveur;
int message;
static Flag_stop flag_stop;

extern void Server_start(){
    flag_stop = OFF;
    Pilot_start();

    socket_ecoute = socket(AF_INET, SOCK_STREAM, 0);
    adresse_serveur.sin_family = AF_INET;
    adresse_serveur.sin_port = htons(PORT_DU_SERVEUR);
    adresse_serveur.sin_addr.s_addr = htonl(INADDR_ANY);

    //printf("%d\n",adresse_serveur.sin_addr.s_addr);

    if(bind(socket_ecoute, (struct sockaddr *) &adresse_serveur, sizeof(adresse_serveur)) == -1){
        printf("Erreur : le socket écoute ne s'est pas lié au serveur.\n");
        exit(0);
    }
    if(listen(socket_ecoute, MAX_PENDING_CONNECTIONS) == -1){
        printf("Erreur : le socket serveur n'a pas pu écouter les connexions entrantes.\n");
        exit(0);
    }
    
    //socket_communication_ext = accept(socket_ecoute, NULL, 0);

}

extern void Server_stop(){
    close(socket_ecoute);
}

extern void Server_sendMsg(int msg){
    //strcpy(message,"Message provenant du serveur");
    //message = htonl(message); /* Pour envoyer la donnée au format du host vers le format du network (32 bits) */
    write(socket_communication_ext, &msg, sizeof(msg));
    close(socket_communication_ext);

    //message = htons(message);

    // while(1){
    //     /* Acceptation de la connexion */
    //     // socket_communication_ext = accept(socket_ecoute, NULL, 0);
    //     if (fork() == 0){     /* fork() : duplication de la tâche, on assigne 0 à la tâche créée */
    //         if(write(socket_communication_ext, &message, sizeof(message)) == -1){   /* ATTENTION : fonction bloquante */
    //             printf("Erreur d'envoi du message par le serveur.\n");
    //             exit(0);
    //         }
    //     }
    // }
}

extern void Server_readMsg(){
    
    // if(read(socket_communication_ext, &message, sizeof(message)) == -1){   /* ATTENTION : fonction bloquante */
    //     printf("Erreur de lecture du message par le client.\n");
    //     exit(0);
    // }
    // printf("Message reçu par le serveur : %s\n",message);

    while(flag_stop == OFF){
        //printf("NOUVEAU WHILE\n");
        /* Acceptation de la connexion */
        socket_communication_ext = accept(socket_ecoute, NULL, 0);
        if (fork() == 0){     /* fork() : duplication de la tâche, on assigne 0 à la tâche créée */
            if(read(socket_communication_ext, &message, sizeof(message)) == -1){   /* ATTENTION : fonction bloquante */
                printf("Erreur de lecture du message envoyé par le client.\n");
                exit(0);
            }
            printf("Message envoyé par le client : %d\n",message);
            Server_give_order();
        }
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


