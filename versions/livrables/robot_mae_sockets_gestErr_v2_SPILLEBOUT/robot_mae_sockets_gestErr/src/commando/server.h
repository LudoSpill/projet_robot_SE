#ifndef SERVER_H
#define SERVER_H

#include "common_data.h"

extern void Server_start();

extern void Server_stop();

extern void Server_readMsg();

extern void handle_error(int err, char err_msg[]);

#endif // SERVER_H
