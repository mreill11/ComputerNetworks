/* Matt Reilly - mreill11
 * CSE 30264 - Programming Assignment 1
 * Simple UDP Client
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/time.h>

#define BUFSIZE 4096

void readFile(char *dest, char *filename);

// error - wrapper for perror
void error(char *msg) {
    perror(msg);
    exit(0);
}

int main(int argc, char **argv) {
    int sockfd, portno, n;
    int serverlen;
    struct sockaddr_in serveraddr;
    struct hostent *server;
    char *hostname;
    char buf[BUFSIZE];
    //clock_t start, stop;
	struct timeval start, end;
	double rtt;

    /* check command line arguments */
    // CHANGE <text or file name>
    if (argc != 4) {
       fprintf(stderr,"usage: %s <hostname> <port> <text or file name>\n", argv[0]);
       exit(0);
    }

    hostname = argv[1];
    portno = atoi(argv[2]);

    /* socket: create the socket */
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) 
        error("ERROR opening socket");

    /* gethostbyname: get the server's DNS entry */
    server = gethostbyname(hostname);
    if (server == NULL) {
        fprintf(stderr,"ERROR, no such host as %s\n", hostname);
        exit(0);
    }

    /* build the server's Internet address */
    bzero((char *) &serveraddr, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, 
	  (char *)&serveraddr.sin_addr.s_addr, server->h_length);
    serveraddr.sin_port = htons(portno);

    /* get a message from the user */
    bzero(buf, BUFSIZE);
    //printf("Please enter msg: ");
    //fgets(buf, BUFSIZE, stdin);
    //buf = argv[3];
	strcpy(buf, argv[3]);
	if (access(buf, F_OK) != -1) {
		// send file
		readFile(buf, buf);
	}

    /* send the message to the server */
    serverlen = sizeof(serveraddr);
    n = sendto(sockfd, buf, strlen(buf), 0, &serveraddr, serverlen);
	gettimeofday(&start, NULL);
	if (n < 0) 
      error("ERROR in sendto");
    
    /* print the server's reply */
    n = recvfrom(sockfd, buf, strlen(buf), 0, &serveraddr, &serverlen);
	gettimeofday(&end, NULL);
	if (n < 0) 
      error("ERROR in recvfrom");
    printf("Echo from server: %s\n", buf);	
 	printf("RTT: %Lf\n",(long double)((end.tv_sec * (int)1e6 + end.tv_usec) - (start.tv_sec * (int)1e6 + start.tv_usec)));
    return 0;
}

void readFile(char *dest, char *filename) {
	FILE *fp = fopen(filename, "r");
	if (fp != NULL) {
		size_t new_len = fread(dest, sizeof(char), BUFSIZE, fp);
		if (ferror(fp) != 0) { 
			fputs("Error reading file", stderr);
		} else {
			dest[new_len++] = '\0';
		}
		fclose(fp);
	}
}
