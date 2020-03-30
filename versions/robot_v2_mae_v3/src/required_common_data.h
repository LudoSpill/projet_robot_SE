#ifndef COMMON_DATA_H
#define COMMON_DATA_H

#define PORT_DU_SERVEUR (12346)
#define MAX_LOGS_LENGTH (128)
#define ERROR_MSG_MAX_LENGTH (80)

#define VEL_POWER_FORWARD (80)
#define VEL_POWER_BACKWARD (60)
#define VEL_POWER_RIGHT (50)
#define VEL_POWER_LEFT (50)
#define VEL_POWER_STOP (0)

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
	C_QUIT,
	C_DISPLAY_LOGS,
	C_CLEAR_LOGS,
	NB_C_Request
} C_Request;

typedef struct
{
    int robot_speed;
    int bump;
    int luminosity;
}Robot_Logs;


#endif /* COMMON_DATA_H */
