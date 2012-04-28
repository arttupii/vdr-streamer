#include<stdio.h>
#include<string.h>	//strlen
#include"CSocket.h"
#include<sys/socket.h>
#include<arpa/inet.h>	//inet_addr
#include<unistd.h>	//write

int create_server(string ip, int port)
{
	int socket_desc , c;
	struct sockaddr_in server , client;
	char *message;

	//Create socket
	socket_desc = socket(AF_INET , SOCK_STREAM , 0);
	if (socket_desc == -1)
	{
		printf("create_server::Could not create socket");
	}
	int xx=1;
	setsockopt(socket_desc, SOL_SOCKET, SO_REUSEADDR, &xx, sizeof(xx));

	//Prepare the sockaddr_in structure
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = inet_addr(ip.c_str());
	server.sin_port = htons( port );
	printf("create_server::%s %d", ip.c_str(), port);
	//Bind
	if( bind(socket_desc,(struct sockaddr *)&server , sizeof(server)) < 0)
	{
		puts("create_server::bind failed");
		return 1;
	}


	//Listen
	listen(socket_desc , 3);
	return socket_desc;
}

int wait_connection(int socket)
{
	int c, new_socket;
	struct sockaddr_in client;

	c = sizeof(struct sockaddr_in);
	new_socket = accept(socket, (struct sockaddr *)&client, (socklen_t*)&c);
	if (new_socket<0)
	{
		perror("wait_connection::accept failed");
		return -1;
	}
	return new_socket;
}

int create_client(string ip, int port)
{
	int socket_desc;
	struct sockaddr_in server;

	//Create socket
	socket_desc = socket(AF_INET , SOCK_STREAM , 0);
	if (socket_desc == -1)
	{
		printf("create_client::Could not create socket");
	}

	server.sin_addr.s_addr = inet_addr(ip.c_str());
	server.sin_family = AF_INET;
	server.sin_port = htons( port );

	//Connect to remote server
	if (connect(socket_desc , (struct sockaddr *)&server , sizeof(server)) < 0)
	{
		puts("create_client::connect error");
		return -1;
	}

	puts("Connected\n");
	return socket_desc;
}

int send_to_socket(int socket, const char *data, int datalength)
{
	int ret;
	ret=send(socket, data , datalength, 0);
	
	return ret;
}

int receive_from_socket(int socket, char *buffer, int length, bool dont_block)
{
	//Receive a reply from the server
	int ret;
	ret=recv(socket, buffer, length , dont_block?MSG_DONTWAIT:0);
	return ret;
}

void close_socket(int socket)
{
	close(socket);
}

