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

#define MAX_PENDING_CONNECTIONS (5)

extern void Server_start();

extern void Server_stop();

extern void Server_sendMsg();

extern void Server_readMsg();

void communication_avec_client(int socket, int age_capitaine){

    DesDonnees ma_donnee;
    int quantite_envoyee;

    strcpy(ma_donnee.message, "bonjour");
    ma_donnee.age_capitaine = htonl(age_capitaine); /* Pour envoyer la donnée au format du host vers le format du network (32 bits) */

    /* On envoie une donnée jusqu'à ce que le client ferme la connexion */
    quantite_envoyee = write(socket, &ma_donnee, sizeof(ma_donnee));

    close(socket);

    exit(0);
}

int main(void){
    int socket_ecoute;
    int socket_donnees;
    int age_capitaine_courant = 25;
    struct sockaddr_in mon_adresse;

    /* Creation du socket : AF_INET = IP, SOCK_STREAM = TCP => man socket pour plus d'infos */
    socket_ecoute = socket(AF_INET, SOCK_STREAM, 0);
    mon_adresse.sin_family = AF_INET;                   /* Type d'adresse = IP */
    mon_adresse.sin_port = htons(PORT_DU_SERVEUR);      /* htons : host to network + s = 2 octets : Port TCP où le service est accessible */
    mon_adresse.sin_addr.s_addr = htonl(INADDR_ANY);    /* htons : host to network + l = 4 octets : On s'attache à toutes les interfaces */
    
    /* On attache le coket à l'adresse indiquée */
    bind(socket_ecoute, (struct sockaddr *)&mon_adresse, sizeof(mon_adresse));  /* sockaddr_in = héritage de sockaddr : une adresse INTERNET (sockaddr_in) est une adresse (sockaddr) */

    /* Mise en écoute du  */
    listen(socket_ecoute, MAX_PENDING_CONNECTIONS); /* MAX_PENDING_CONNECTIONS : nb max de connexions simultanées */

    while(1){
        /* Acceptation de la connexion */
        socket_donnees = accept(socket_ecoute, NULL, 0);
        age_capitaine_courant ++;
        /* On crée un tâche qui va communiquer avec le client */
        if (fork() == 0){                                           /* fork() : duplication de la tâche, on assigne 0 à la tâche créée */
            communication_avec_client(socket_donnees, age_capitaine_courant);
        }
    }

    /* On ferme le port sur lequel on écoutait (ne sert à rien) */
    close(socket_ecoute);

}