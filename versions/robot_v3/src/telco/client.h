#ifndef CLIENT_H
#define CLIENT_H

#include "../common_data.h"

extern int Client_start(char * myIp);

extern void Client_stop();

extern void Client_sendMsg(int);

extern PilotState Client_readLog();

#endif // CLIENT_H
