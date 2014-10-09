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


#define UINT64_MAX (__UINT64_C(18446744073709551615))

uint64_t htonll(uint64_t hll){
	if(htons(42)!=42){
		return (((uint64_t) htonl(hll)) << 32) + htonl(hll >> 32);
	}
	else
		return hll;
}
uint64_t ntohll(uint64_t nll){
	if(htons(42)!=42){
		return (((uint64_t) ntohl(nll)) << 32) + ntohl(nll >> 32);
	}
	else
		return nll;
}
uint8_t htonb(uint8_t hb){
	if(htons(42)!=42){
		return htons((uint16_t)hb)>>8;
	}
	else
		return hb;
}
uint8_t ntohb(uint8_t nb){
	if(htons(42)!=42){
		return ntohs((uint16_t)nb)>>8;
	}
	else
		return nb;
}

int sockterm = 0;

int sockTcp=-1;
pthread_t sockThreadTcp=0;
struct sockaddr_in sockaddrTcp;

int bConnected = 0;


void CloseSockCS(int sock){
	if(sockTcp>=0)
		close(sockTcp);
}


void* SocketThread(){
	//#define	ECONNREFUSED	111	/* Connection refused */
	//#define	EBADF		 9	/* Bad file number */
	while(!sockterm){

		printf("\e[32mAttempting connection to server...\e[m\n");
		if(connect(sockTcp, &sockaddrTcp, sizeof(sockaddrTcp))<0){
			printf("\e[1;33mSocket connection error %d\e[m\n", errno);
		}
		else
		{
			bConnected = 1;
			printf("\e[32mConnected to server\e[m\n");

			//On lit ce qu'il raconte
			while(!sockterm){

				//Réception des données
				char buffer[sizeof(struct Event)];
				int nReceivedBytes=recv(sockTcp, buffer, sizeof(buffer), MSG_WAITALL);
				if(nReceivedBytes>0){
					struct Event* ev = ((struct Event*)(buffer));
					ev->id = ntohb(ev->id);
					ev->data[0] = ntohll(ev->data[0]);
					ev->data[1] = ntohll(ev->data[1]);
					SocketOnEventReceived(*ev);
				}
				else
					break;
			}

			close(sockTcp);
			bConnected = 0;
			printf("\e[1;33mDisconnected from server\e[m\n");
		}
		sleep(1);

	}
	return 0;
}


int SocketInit(const char* server){

	//Init de la socket
	sockTcp = socket(AF_INET, SOCK_STREAM, 0);
	if(sockTcp < 0){
		printf("\e[1;31;43mUnable to init TCP socket: err %d\e[m\n", errno);
		return sockTcp;
	}

	//Préparation de l'adresse côté serveur
	sockaddrTcp.sin_addr.s_addr = inet_addr(server);
	sockaddrTcp.sin_family = AF_INET;
	sockaddrTcp.sin_port = htons(1338);

	printf("\e[32mTCP socket init done\e[m\n");
	return 0;
}


void SocketStart(){
	sockterm = 0;

	int created = pthread_create(&sockThreadTcp, NULL, SocketThread, NULL);
	if(created<0){
		printf("\e[1;31;43mUnable to start TCP socket thread\e[m\n");
	}
}

void SocketClose(){
	printf("\e[33mClosing sockets\e[m\n");
	CloseSockCS(sockTcp);
	sockterm = 1;

	printf("\e[33mSockets closed\e[m\n");
}


float ConvertToBatteryValue(uint64_t data[2]){
	return 10*(data[0]/(double)UINT64_MAX);
}


double ConvertToCompassValue(uint64_t data[2]){
	return 360.0*(data[0]/(double)UINT64_MAX);
}
double ConvertToWindDirValue(uint64_t data[2]){
	return 360.0*(data[0]/(double)UINT64_MAX)-180.0;
}
double ConvertToRollValue(uint64_t data[2]){
	return 360.0*(data[0]/(double)UINT64_MAX)-180.0;
}
struct GpsCoord ConvertToGpsValue(uint64_t data[2]){
	struct GpsCoord ret = {
		(180.0*(data[0]/(double)UINT64_MAX))-90.0,
		(380.0*(data[1]/(double)UINT64_MAX))-180.0
	};
	return ret;
}


void SocketSendEvent(struct Event ev){
	if(sockTcp>=0 && bConnected){
		//Convert TCP data to network bit order
		ev.id = htonb(ev.id);
		ev.data[0] = htonll(ev.data[0]);
		ev.data[1] = htonll(ev.data[1]);
		//Send
		send(sockTcp, &ev, sizeof(ev), MSG_DONTWAIT);
	}
}

void SocketSendSail(unsigned short value){
	struct Event ev = {
		DEVICE_ID_SAIL,
		{htonll(value), 0}
	};
	SocketSendEvent(ev);
}


void SocketSendHelm(float value){
	struct Event ev = {
		DEVICE_ID_HELM,
		{htonll((value+45.0)*(UINT64_MAX/90.0)), 0}
	};
	SocketSendEvent(ev);
}
