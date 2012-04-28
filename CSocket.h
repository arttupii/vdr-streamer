#ifndef _CSOCKET_H_
#define _CSOCKET_H_
#include "errno.h"
#include<string>


using namespace std;


int create_server(string ip, int port);

int wait_connection(int socket);

int create_client(string ip, int port);

int send_to_socket(int socket, const char *data, int datalength);

int receive_from_socket(int socket, char *buffer, int length, bool dont_block=false);

void close_socket(int socket);

#endif
