#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>

#include <sys/types.h>
#include <sys/uio.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/in.h>

#ifndef FD_COPY
#define FD_COPY(f, t)   (void)(*(t) = *(f))
#endif

/* data structure for tracking each client */
struct client_state {
	struct client_state *prev;
	struct client_state *next;
	int message_count;

	// added
	int byte_count;
	char name[255];
	// added

	int socket;
};

struct client_state *clients=0;
int client_count = 0;

// added
void broadcast_msg(char* msg, struct client_state* from) {
	struct client_state *c = clients;
	while(c) {
		if(c!=from) 
			send(c->socket, msg, strlen(msg)+1, 0); // +1 for the null terminator!
		c=c->next;
	}
}

void unicast_msg(char* msg, char* recipient) {
	struct client_state *c = clients;
	while(c) {
		if(strcmp(c->name, recipient)==0) 
			send(c->socket, msg, strlen(msg)+1, 0); // +1 for the null terminator!
		c=c->next;
	}
}

void handle_command(char* cmd, struct client_state* client) {
	char temp_name[255];
	char message[255];
  char recipient[255];
	char send_msg[500];

	if(sscanf(cmd, "name %s\r\n", temp_name)) {
		
		sprintf(send_msg, "%s set name to %s\n", client->name, temp_name);
		printf("%s",send_msg);		
		strcpy(client->name, temp_name);

		broadcast_msg(send_msg, client);
	}	
	else if(sscanf(cmd, "say %[^\r\n]", message)) {		
		printf("%s said: %s\n",client->name,message);

		sprintf(send_msg, "%s> %s\n", client->name, message);
		broadcast_msg(send_msg,client);
	}
	else if(sscanf(cmd, "whisper %[^ ] %[^\r\n]", recipient, message)==2) {		
		printf("%s whispered to %s: %s\n",client->name,recipient,message);

		sprintf(send_msg, "~%s> %s\n", client->name, message);
		unicast_msg(send_msg,recipient);
	}
}
// added

int main(int argc, char **argv) {	
  int port;	
  if(argc==2) {
		port=atoi(argv[1]);
	}
	setlinebuf(stdout);	
	

	int server_sock = socket(AF_INET, SOCK_STREAM, 0);
	if(server_sock < 0) {
		perror("Creating socket failed: ");
		exit(1);
	}
	
	struct sockaddr_in addr; 	// internet socket address data structure
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port); // byte order is significant
	addr.sin_addr.s_addr = INADDR_ANY; // listen to all interfaces

	int yes=1;
  if (setsockopt(server_sock, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
    perror("setsockopt");
    exit(1);
  }

	int res = bind(server_sock, (struct sockaddr*)&addr, sizeof(addr));
	if(res < 0) {
		perror("Error binding to port");
		exit(1);
	}

	if (listen (server_sock, 1) < 0) { perror ("listen"); exit(1); } 

	/* initializing data structure for select call */
	fd_set readset;
	FD_ZERO(&readset);
	FD_SET(server_sock,&readset);	

	while(1) {
		fd_set rdyset;
		FD_COPY(&readset,&rdyset);
		int rdy = select(FD_SETSIZE,&rdyset,0,0,0);

		/* if the server_sock has a new connection coming in, accept it */
		if(FD_ISSET(server_sock,&rdyset)) {
			int sock;
			struct sockaddr_in remote_addr;
			unsigned int socklen = sizeof(remote_addr); 
			
			sock = accept(server_sock, (struct sockaddr*)&remote_addr, &socklen);
			if(res < 0) { perror("Error accepting connection"); exit(1); }

			/* allocate and initialize new client state */
			struct client_state *state=
				(struct client_state*)malloc(sizeof(struct client_state));
			state->next=clients;
			if(clients)
				clients->prev=state;
			state->prev=0;
			state->message_count=0;
			clients=state;
			state->socket=sock;
			client_count++;

			// added
			sprintf(state->name,"Client %d",client_count);
			printf("Got connection (%s)\n",state->name);
			// added

			/* add new socket to fd_set for select */
			FD_SET(sock,&readset);
		}

		/* if any of the active clients are ready to deliver a message,
			 read it and print it */
		struct client_state *clstate = clients;
		while(clstate) {
			if(FD_ISSET(clstate->socket,&rdyset)) {
				char buf[255];
				memset(buf,0,255);
				int rec_count = recv(clstate->socket,buf,255,0);
				if(rec_count > 0) {
					clstate->message_count++;

					// added
					clstate->byte_count+=rec_count;
					handle_command(buf,clstate);
					// added
				}
				/* if we got nothing, that means the connection is closed */
				else {
					printf("closing connection...\n");
					if(clstate->prev == 0) {
						clients = clstate->next;
						if(clients)
							clients->prev = 0;
					}
					else {
						clstate->prev->next = clstate->next;						
						if(clstate->next) 
							clstate->next->prev = clstate->prev;
					}					
					client_count--;

					// tell the others
					char send_msg[255];
					sprintf(send_msg, "%s disconnected.\n", clstate->name);
					broadcast_msg(send_msg, clstate);
					printf("%s disconnected. A total of %d bytes received from %s.\n",clstate->name, clstate->byte_count, clstate->name);

					// clean up the rest of the state
					shutdown(clstate->socket,SHUT_RDWR);
					close(clstate->socket);						
					FD_CLR(clstate->socket,&readset);
					struct client_state *tofree = clstate;
					clstate = clstate->next;
					free(tofree);					
					continue;
				}
			} // end if(FD_ISSET...
			clstate=clstate->next;
		} // end while
	}
	shutdown(server_sock,SHUT_RDWR);
}
