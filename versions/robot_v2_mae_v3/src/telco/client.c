#include "required_common_data.h"
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include "client.h"
#include "remoteui.h"

#define ERROR_MSG_MAX_LENGTH (80)

int socket_reseau;
struct sockaddr_in adresse_serveur;
char err_msg[ERROR_MSG_MAX_LENGTH];

/**
 * Passer la fct en int et renvoyer 
 */
extern void Client_start(){
    socket_reseau = socket(AF_INET, SOCK_STREAM, 0);
    /* handle error */
        if(errno != 0){
            strcpy(err_msg, "Erreur de création du socket...");
            handle_error(errno, err_msg);
        }
    adresse_serveur.sin_family = AF_INET;
    adresse_serveur.sin_port = htons(PORT_DU_SERVEUR);
    adresse_serveur.sin_addr.s_addr = htonl(INADDR_ANY);

    /* On demande la connexion auprès du serveur */
    if(connect(socket_reseau, (struct sockaddr *)&adresse_serveur, sizeof(adresse_serveur)) == -1){ //regarder doc connect
        /* handle error */
        if(errno != 0){
            strcpy(err_msg, "Erreur de connexion du client au serveur...");
            handle_error(errno, err_msg);
        }
    }
    else
    {
        errno = 0;
        char err_msg[] = "Connexion au serveur réussie...";
        handle_error(errno, err_msg);
    }
}

extern void Client_stop(){
    close(socket_reseau);
    /* handle error */
        if(errno != 0){
            strcpy(err_msg, "Erreur de fermeture du client...");
            handle_error(errno, err_msg);
        }
}

extern void Client_sendMsg(int msg){
    if(write(socket_reseau, &msg, sizeof(msg)) == -1){
        /* handle error */
        if(errno != 0){
            strcpy(err_msg, "Erreur d'envoi du message par le client...");
            handle_error(errno, err_msg);
        }
    }
}

extern Robot_Logs Client_readLog(){
    char logs_string[MAX_LOGS_LENGTH];
    if(read(socket_reseau, &logs_string, sizeof(logs_string)) == -1){   /* ATTENTION : fonction bloquante */
        /* handle error */
        if(errno != 0){
            strcpy(err_msg, "Erreur de lecture du message par le client...");
            handle_error(errno, err_msg);
        }
    }
    int robot_speed=0, robot_collision=0, robot_luminosity=0;
    sscanf(logs_string,"%d %d %d", &robot_speed, &robot_collision, &robot_luminosity);

    Robot_Logs logs_int ={
        robot_speed,robot_collision,robot_luminosity
    };

    return logs_int;
}
