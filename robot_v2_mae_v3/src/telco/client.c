
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

static void Client_connect();
static void Client_disconnect();

int un_socket;
struct sockaddr_in adresse_du_serveur;

extern void Client_start(){
    
    /* on choisit un socket TCP (SOCK_STREAM sur IP (AF_INET)) */
    un_socket = socket(AF_INET, SOCK_STREAM, 0);

    /* AF_INET = famille TCP/IP */
    adresse_du_serveur.sin_family = AF_INET;

    /* port du serveur auquel se connecter */
    adresse_du_serveur.sin_port = htons(PORT_DU_SERVEUR);

    /* adresse IP (ou nom de domaine) du serveur auquel se connecter */
    adresse_du_serveur.sin_addr = *((struct in_addr *)gethostbyname("localhost")->h_addr_list[0]);
    
     /* On demande la connexion auprès du serveur */
    connect(un_socket, (struct sockaddr *)&adresse_du_serveur, sizeof(adresse_du_serveur));

}

extern void Client_stop(){
    close(un_socket);
}

extern void Client_sendMsg(){

}

extern void Client_readMsg(){
    read(un_socket, &donnees, sizeof(donnees));     /* ATTENTION : fonction bloquante */
}

extern void Fct_salut(int coucou){}









