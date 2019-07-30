#include <string>

using namespace std;

#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 

#define BUFF_SIZE   100 // buffer size

void error(string msg) {
    perror(msg.c_str());
    exit(0);
}

int main(int argc, char *argv[]) {
    int sockfd, portno, n;

    struct sockaddr_in serv_addr;
    struct hostent *server;

    char buffer[BUFF_SIZE];
    int request_type = 1;
    const int STOP_CONNECTION = 4;
    
    if (argc < 3) {
        fprintf(stderr, "usage %s hostname port\n", argv[0]);
        exit(0);
    }
    portno = atoi(argv[2]);
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
        error("ERROR opening socket");
    server = gethostbyname(argv[1]);
    if (server == NULL) {
        fprintf(stderr, "ERROR, no such host\n");
        exit(0);
    }
    bzero((char *) &serv_addr, sizeof (serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *) server->h_addr,
            (char *) &serv_addr.sin_addr.s_addr,
            server->h_length);
    serv_addr.sin_port = htons(portno);
    if (connect(sockfd, (struct sockaddr *) &serv_addr, sizeof (serv_addr)) < 0)
        error("ERROR connecting");

    // continue to send request until the user wants to stop it 
    while (request_type != STOP_CONNECTION) {
        printf("Please enter the message: ");
        bzero(buffer, BUFF_SIZE);
        fgets(buffer, BUFF_SIZE - 1, stdin);
        n = write(sockfd, buffer, strlen(buffer));
        if (n < 0)
            error("ERROR writing to socket");
        
        sscanf(buffer, "%d", &request_type);        
        bzero(buffer, BUFF_SIZE);
        n = read(sockfd, buffer, BUFF_SIZE - 1);
        if (n < 0)
            error("ERROR reading from socket");
        printf("%s\n", buffer);
    }


    return 0;
}