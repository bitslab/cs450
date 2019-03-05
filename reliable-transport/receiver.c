#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <signal.h>

void handle_alarm(int sig) {
	fprintf(stderr,"More than a second passed after the last packet. Exiting.\n");
	exit(1);
}

int main(int argc, char** argv) {	
	
	if(argc<2) {		
		printf("Usage: ./receiver <port>\n");
		exit(1);
	}
	signal(SIGALRM,handle_alarm);
	
	int sock = socket(AF_INET, SOCK_DGRAM, 0);
	if(sock < 0) {
		perror("Creating socket failed: ");
		exit(1);
	}
	
	struct sockaddr_in addr; 	// internet socket address data structure
	addr.sin_family = AF_INET;
	addr.sin_port = htons(atoi(argv[1])); // byte order is significant
	addr.sin_addr.s_addr = INADDR_ANY;
	
	int res = bind(sock, (struct sockaddr*)&addr, sizeof(addr));
	if(res < 0) {
		perror("Error binding: ");
		exit(1);
	}

	char buf[255];
	memset(&buf,0,sizeof(buf));

	while(1) {
		int recv_count = recv(sock, buf, 255, 0);

		// stop receiving after a second has passed
		alarm(1);
		
		if(recv_count<0) { perror("Receive failed");	exit(1); }
		write(1,buf,recv_count);
	}
	
	shutdown(sock,SHUT_RDWR);
}


