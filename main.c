#include <stdio.h>
#include <stdlib.h>

#include "socket.h"


void SocketOnEventReceived(struct Event ev){
	printf("Received something\n");

}

int main()
{

	int error = SocketInit("127.0.0.1");
	if(error==0){

		//Start client handling thread
		SocketStart();


		int running=1;

		//Main loop
		while(running){
			sleep(1);

			SocketSendHelm(5.0);


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
