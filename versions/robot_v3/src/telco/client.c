#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <pthread.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include "client.h"
#include "remoteui.h"
#include "watchdog.h"

#define ERROR_MSG_MAX_LENGTH (80)
#define CONNECTION_TIMEOUT (3)

static void Client_start_connect();
static void * Client_try_connect(void *arg);
static void abortConnection();
static void waitEndOfConnectThread();

int socket_reseau;
struct sockaddr_in adresse_serveur;
char err_msg[ERROR_MSG_MAX_LENGTH];
Flag flag_timeout_connexion;
pthread_t thread_connection;

int connect_error_code;

/**
 * Passer la fct en int et renvoyer 
 */
extern int Client_start(char * myIp){
    socket_reseau = socket(AF_INET, SOCK_STREAM, 0);
    /* handle error */
    if(errno != 0){
        strcpy(err_msg, "Erreur de création du socket...");
        handle_error(errno, err_msg);
    }
    
    adresse_serveur.sin_family = AF_INET;
    adresse_serveur.sin_port = htons(PORT_DU_SERVEUR);
    adresse_serveur.sin_addr.s_addr = inet_addr(myIp);
    // adresse_serveur.sin_addr.s_addr = htonl(INADDR_ANY);


    /* On demande la connexion auprès du serveur avec un watchdog pour timeout */

    Client_start_connect();
    Watchdog * watchdog_connection = Watchdog_construct(CONNECTION_TIMEOUT,abortConnection);
    Watchdog_start(watchdog_connection);
    waitEndOfConnectThread();
    Watchdog_cancel(watchdog_connection);
    Watchdog_destroy(watchdog_connection);
    
    return connect_error_code;
}

static void Client_start_connect(){
    flag_timeout_connexion = OFF;
    int err = pthread_create(&thread_connection,NULL, &Client_try_connect,NULL);
    if(err != 0){
        strcpy(err_msg, "Erreur de création du thread de connexion...");
        handle_error(errno, err_msg);
    }
}

static void * Client_try_connect(void * unused){
    // Pour rendre la socket non bloquante-------------------------------------
    long arg;
    if( (arg = fcntl(socket_reseau, F_GETFL, NULL)) < 0) { 
        fprintf(stderr, "Error fcntl(..., F_GETFL) (%s)\n", strerror(errno)); 
        exit(0); 
    } 
    arg |= O_NONBLOCK; 
    if( fcntl(socket_reseau, F_SETFL, arg) < 0) { 
        fprintf(stderr, "Error fcntl(..., F_SETFL) (%s)\n", strerror(errno)); 
        exit(0); 
    }
    //--------------------------------------------------------------------------
    do{
        connect_error_code = connect(socket_reseau, (struct sockaddr *)&adresse_serveur, sizeof(adresse_serveur));
        if(connect_error_code == 0){ 
            errno = 0;
            char err_msg[] = "Connexion au serveur réussie...";
            handle_error(errno, err_msg);
        }
    } while(flag_timeout_connexion == OFF && connect_error_code != 0);

    // Pour rendre la socket à nouveau bloquante------------------------------
     if( (arg = fcntl(socket_reseau, F_GETFL, NULL)) < 0) { 
        fprintf(stderr, "Error fcntl(..., F_GETFL) (%s)\n", strerror(errno)); 
        exit(0); 
    } 
    arg &= (~O_NONBLOCK); 
    if( fcntl(socket_reseau, F_SETFL, arg) < 0) { 
        fprintf(stderr, "Error fcntl(..., F_SETFL) (%s)\n", strerror(errno)); 
        exit(0); 
    } 
    //--------------------------------------------------------------------------

    return NULL;
}

static void waitEndOfConnectThread(){
    pthread_join(thread_connection,NULL);
}

static void abortConnection(){
    flag_timeout_connexion = ON;
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
    //trace("Envoi message par le client : %d\n",msg);
    if(write(socket_reseau, &msg, sizeof(msg)) == -1){
        /* handle error */
        if(errno != 0){
            strcpy(err_msg, "Erreur d'envoi du message par le client...");
            handle_error(errno, err_msg);
        }
    }
}

extern PilotState Client_readLog(){
    trace("J'essaie de read\n");
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

    PilotState logs_int ={
        robot_speed,robot_collision,robot_luminosity
    };
    
    trace("J'ai réussi à read\n");

    return logs_int;
}
