#ifndef CLIENT_H
#define CLIENT_H

#include "common_data.h"

extern void Client_start();

extern void Client_stop();

extern void Client_sendMsg(int);

extern Robot_Logs Client_readLog();

#endif // CLIENT_H
