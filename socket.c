#include "socket.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/un.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <unistd.h>
#include <errno.h>







void SocketHandleReceivedEvent(struct Event ev){
	printf("Received something\n");

}










//===================================================================

#define UINT64_MAX (__UINT64_C(18446744073709551615))


int term;

struct sockcs{
	int type;
	int client;
	int server;
};

struct sockcs* sockcsTcp=NULL;
pthread_t sockThreadTcp=0;
struct sockaddr_in sockaddrTcp;


void CloseSockCS(struct sockcs* sock){
	if(sock!=NULL){
		if(sock->client>=0)
			close(sock->client);
		if(sock->server>=0)
			close(sock->server);

		free(sock);
	}
}


void* SocketThread(void* args){
	struct sockcs* sock = (struct sockcs*)args;

	while(!term){
		//On attend qu'un client se connecte au serveur
		printf("\e[32mWaiting for client on %s socket...\e[m\n", sock->type==AF_UNIX?"UNIX":"TCP");
		sock->client = accept(sock->server, NULL, NULL);
		if(sock->client<0){
			if(term) break;
			printf("\e[1;33mAccept error on %s socket\e[m\n", sock->type==AF_UNIX?"UNIX":"TCP");
			continue;
		}
		printf("\e[32mClient is connected on %s socket\e[m\n", sock->type==AF_UNIX?"UNIX":"TCP");

		//On lit ce qu'il raconte
		while(!term){

			//Réception des données
			char buffer[32];
			int nReceivedBytes=recv(sock->client, buffer, sizeof(buffer), MSG_WAITALL);
			if(nReceivedBytes>0){
				SocketHandleReceivedEvent(*((struct Event*)(buffer)));
			}
			else
				break;
		}
		sock->client = -1;
		printf("\e[1;33mClient closed connection on %s socket\e[m\n", sock->type==AF_UNIX?"UNIX":"TCP");
	}
	return 0;
}


int SocketInit(const char* server){
	//TCP/IP SOCKET
	sockcsTcp = (struct sockcs*)malloc(sizeof(struct sockcs));
	sockcsTcp->type = AF_INET;

	//Init de la socket
	sockcsTcp->client = socket(AF_INET, SOCK_STREAM, 0);
	if(sockcsTcp->client < 0){
		printf("\e[1;31;43mUnable to init TCP socket: err %d\e[m\n", errno);
		return sockcsTcp->client;
	}

	//Préparation de l'adresse côté serveur
	sockaddrTcp.sin_addr.s_addr = inet_addr(server);
	sockaddrTcp.sin_family = AF_INET;
	sockaddrTcp.sin_port = htons(1338);

	printf("\e[32mTCP socket init done\e[m\n");
	return 0;
}





float ConvertToBatteryValue(uint64_t data[2]){
	return 10*(data[0]/UINT64_MAX);
}


double ConvertToCompassValue(uint64_t data[2]){
	return 360*(data[0]/UINT64_MAX);
}
double ConvertToWindDirValue(uint64_t data[2]){
	return 360*(data[0]/UINT64_MAX)-180;
}
double ConvertToRollValue(uint64_t data[2]){
	return 360*(data[0]/UINT64_MAX)-180;
}
GpsCoord ConvertToGpsValue(uint64_t data[2]){
	struct GpsCoord ret = {
		(180.0*(data[0]/UINT64_MAX))-90.0,
		(380.0*(data[1]/UINT64_MAX))-180.0
	};
	return ret;
}
