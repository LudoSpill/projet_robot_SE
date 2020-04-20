#ifndef COMMON_DATA_H
#define COMMON_DATA_H

#define trace(fmt, ...) do {fprintf (stderr, "%s:%d:%s(): " fmt, __FILE__, __LINE__, __func__, ##__VA_ARGS__); fflush (stderr); } while (0)

#define IP_DU_SERVEUR ("127.0.0.1")
#define PORT_DU_SERVEUR (12346)
#define MAX_LOGS_LENGTH (128)
#define ERROR_MSG_MAX_LENGTH (80)
#define IP_MAX_LENGTH (15)
#define MQ_MAX_MESSAGES (10)

#define VEL_POWER_FORWARD (80)
#define VEL_POWER_BACKWARD (60)
#define VEL_POWER_RIGHT (50)
#define VEL_POWER_LEFT (50)
#define VEL_POWER_STOP (0)

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef enum {ON, OFF} Flag;

typedef enum {
    LEFT=0, 
    RIGHT, 
    FORWARD, 
    BACKWARD, 
    STOP, 
    QUIT, 
    NB_DIR
} Direction;

typedef enum{
	C_NULL = 0,
	C_FORWARD,
	C_BACKWARD,
	C_RIGHT,
	C_LEFT,
	C_STOP,
    C_DISPLAY_MAIN,
	C_QUIT,
	C_DISPLAY_LOGS,
	C_ES,
	NB_C_Request
} C_Request;


typedef struct
{
    int speed;
    int collision;
    int luminosity;
} PilotState;

extern void handle_error(int err, char err_msg[]);

#endif /* COMMON_DATA_H */
