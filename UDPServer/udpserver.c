// Programming 2 - Simple UDP Server
// Matt Reilly
// nmreill11

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/time.h>

#define BUFSIZE 4096

// error handling
void error(char *msg) {
  perror(msg);
  exit(1);
}

int main(int argc, char **argv) {
	int sockfd; // socket 
	int port; // port number
	int clientlen; // byte size of client's address
	struct sockaddr_in serveraddr; // server's addr
	struct sockaddr_in clientaddr; // client addr
	struct hostent *hostp; // client host info
	struct timeval timestruct;  
	time_t curTime;
	char time[30];
	char buf[BUFSIZE]; // message buffer
	char key[BUFSIZE]; // decryption key
	char *hostaddrp; // dotted decimal host addr string
	int optval; // flag value for setsockopt
	int n, k; // n = message size, k = key size

	// parse command line arguments
	if (argc != 3) {
		fprintf(stderr, "usage: %s <port> <encryption key>\n", argv[0]);
		exit(1);
	}

	port = atoi(argv[1]);
	strcpy(key, argv[2]);

	// create the parent socket
	sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	if (sockfd < 0) 
		error("ERROR opening socket");

	// setsockopt: rerun server faster
	optval = 1;
	setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (const void *)&optval, sizeof(int));

	// build server internet address
	bzero((char *) &serveraddr, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
	serveraddr.sin_port = htons((unsigned short)port);

	// bind the parent socket and port
	if (bind(sockfd, (struct sockaddr *) &serveraddr, sizeof(serveraddr)) < 0) 
		error("ERROR on binding");

	// Wait for message, send response
	clientlen = sizeof(clientaddr);
	while (1) {

		// receive a datagram from a client
		bzero(buf, BUFSIZE);
		n = recvfrom(sockfd, buf, BUFSIZE, 0,(struct sockaddr *) &clientaddr, &clientlen);

		// attempt to get the timestamp when the message is received.

		timestructofday(&timestruct, NULL);
		curTime = (timestruct.tv_sec + timestruct.tv_usec);
		strftime(time,30,"%m-%d-%Y  %T.",localtime(&curTime));
		printf("%s%ld\n",time,timestruct.tv_usec);

		if (n < 0)
			error("ERROR in recvfrom");

		// identify client
		hostp = gethostbyaddr((const char *)&clientaddr.sin_addr.s_addr, sizeof(clientaddr.sin_addr.s_addr), AF_INET);
		if (hostp == NULL)
			error("ERROR on gethostbyaddr");
		hostaddrp = inet_ntoa(clientaddr.sin_addr);
		if (hostaddrp == NULL)
			error("ERROR on inet_ntoa\n");
		printf("server received datagram from %s (%s)\n", hostp->h_name, hostaddrp);
		printf("server received %d/%d bytes: %s\n", strlen(buf), n, buf);

		// sendto: echo the input back to the client
		// encrypt the key
		int keyLength = strlen(key);
		for (i = 0; i < strlen(buf); i++) {
			buf[i] = buf[i] ^ key[i%keyLength];
		}

		n = sendto(sockfd, buf, strlen(buf), 0, (struct sockaddr *) &clientaddr, clientlen);
		k = sendto(sockfd, key, strlen(key), 0, (struct sockaddr *) &clientaddr, clientlen);
		if (n < 0)  {
			error("ERROR in sendto");
		} else if (k < 0) {
			error("ERROR in keysendto");
		}
	}
}