#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <arpa/inet.h>

int main(int argc, char **argv) {	
	if(argc<4) {
		printf("Usage: ./sender <ip> <port> <filename>\n");
	}
	int sock = socket(AF_INET, SOCK_DGRAM, 0);
	if(sock < 0) {
		perror("Creating socket failed: ");
		exit(1);
	}
	
	struct sockaddr_in addr; 	// internet socket address data structure
	addr.sin_family = AF_INET;
	addr.sin_port = htons(atoi(argv[2])); // byte order is significant
	inet_pton(AF_INET,argv[1],&addr.sin_addr.s_addr);
	
	char buf[255];
	memset(&buf,0,sizeof(buf));
	FILE *f=fopen(argv[3],"r");
	if(!f) {
		perror("problem opening file");
	}

	while(fgets(buf,255,f)) {
		int send_count = sendto(sock, buf, strlen(buf), 0,
														(struct sockaddr*)&addr,sizeof(addr));
		if(send_count<0) { perror("Send failed");	exit(1); }
	}	
	shutdown(sock,SHUT_RDWR);
	close(sock);
}


