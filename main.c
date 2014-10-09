#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>

#include "socket.h"


void SocketOnEventReceived(struct Event ev){
	switch(ev.id){
		case DEVICE_ID_BATTERY: 	printf("\t\tReceived Battery=%fv\n", ConvertToBatteryValue(ev.data)); break;
		case DEVICE_ID_COMPASS: 	printf("\t\tReceived Compass=%f°\n", ConvertToCompassValue(ev.data)); break;
		case DEVICE_ID_GPS: 		printf("\t\tReceived GPS=%.8f° ; %.8f°\n", ConvertToGpsValue(ev.data).lat, ConvertToGpsValue(ev.data).lon); break;
		case DEVICE_ID_ROLL: 		printf("\t\tReceived Roll=%f°\n", ConvertToRollValue(ev.data)); break;
		case DEVICE_ID_WINDDIR: 	printf("\t\tReceived Wind=%f°\n", ConvertToWindDirValue(ev.data)); break;

		default: printf("\t\tReceived \e[1;33mUnhandled device id=%d\e[m\n", ev.id);
	}
}

int running=1;
void term(int signum)
{
    printf("Received SIGINT, exiting...\n");
    running = 0;
}

int main(int argc, char** argv)
{

	//Setup interrupt signal handling
	struct sigaction action;
	memset(&action, 0, sizeof(struct sigaction));
	action.sa_handler = term;
	sigaction(SIGINT, &action, NULL);


	char* addr;
	if(argc==2){
		addr = argv[1];
	}
	else{
		printf("Usage: ./tcpapi server_address\n");
		return -1;
	}

	srand(time(NULL));

	int error = SocketInit(addr);
	if(error==0){

		//Start client handling thread
		SocketStart();


		//Main loop
		while(running){

			sleep(1);

			switch(rand()%4){
				case 0:
				{
					float fValue = rand()%900 / 10.0 - 45.0;
					SocketSendHelm(fValue);
					printf("Sending Helm=%f\n", fValue);
					break;
				}
				case 1:
				{
					int nValue = rand()%256;
					SocketSendSail(nValue);
					printf("Sending Sail=%d\n", nValue);
					break;
				}
			}
		}

		//Close sockets
		SocketClose();
	}
	else{
		printf("Unable to init socket ! Error code %d",error);
		return -1;
	}

    return 0;
}
