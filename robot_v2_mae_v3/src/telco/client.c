
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

int socket_reseau;
struct sockaddr_in adresse_serveur;
int message;

extern void Client_start(){
    socket_reseau = socket(AF_INET, SOCK_STREAM, 0);
    adresse_serveur.sin_family = AF_INET;
    adresse_serveur.sin_port = htons(PORT_DU_SERVEUR);
    adresse_serveur.sin_addr.s_addr = htonl(INADDR_ANY);

        printf("%d\n",adresse_serveur.sin_addr.s_addr);

    //adresse_serveur.sin_addr = *((struct in_addr *)gethostbyname("localhost")->h_addr_list[0]);

    /* On demande la connexion auprès du serveur */
    if(connect(socket_reseau, (struct sockaddr *)&adresse_serveur, sizeof(adresse_serveur)) == -1){
        printf("Erreur de connexion du client au serveur...\n");
        exit(0);
    }
    else
    {
        printf("Connexion au serveur réussie...\n");
    }
    

}

extern void Client_stop(){
    close(socket_reseau);
}

extern void Client_sendMsg(int msg){

    //strcpy(msg,"Message provenant du client");
    //message = htonl(message); /* Pour envoyer la donnée au format du host vers le format du network (32 bits) */

    write(socket_reseau, &msg, sizeof(msg));

    //exit(0);
}

extern void Client_readMsg(){
    if(read(socket_reseau, &message, sizeof(message)) == -1){   /* ATTENTION : fonction bloquante */
        printf("Erreur de lecture du message par le client.\n");
        exit(0);
    }
    printf("Message reçu par le client : %d\n",message);

    
}








