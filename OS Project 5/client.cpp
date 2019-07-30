#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <unistd.h>
#include <math.h>
#include <time.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <cstdlib>
#include <cstring>
#include <sys/wait.h>
#define bzero(b,len) (memset((b), '\0', (len)), (void) 0)

using namespace std;

typedef struct {
	int id;
	int sId;
} Config;

char IP[80];
int PORT;

void printArray(int ar[], int size) {
	for (int i = 0; i < size; i++) {
		if (ar[i] >= 0) {
			printf("%d", ar[i]);
		} else {
			printf("%d", ar[i]);
		}
		printf(" ");
	}
	printf("\n");

}

void decode(int EM[], int w[], int id) {
	printf("\nChild %d\n", id);
	printf("Signal:");
	printArray(EM, 12);
	printf("Code:");
	printArray(w, 4);

	int c = 0;
	int D[12];
	int bin[3];
	int sum = 0;
	int value = 0;
	for (int i = 0; i < 12; i++) {
		D[i] = EM[i] * w[ i % 4];
		sum += D[i];
		if (i == 3) {
			value = sum / 4;
			if (value < 0) {
				bin[0] = 0;
			} else {
				bin[0] = value;
			}
			sum = 0;
		}

		if (i == 7) {
			value = sum / 4;
			if (value < 0) {
				bin[1] = 0;
			} else {
				bin[1] = value;
			}
			sum = 0;
		}

		if (i == 11) {
			value = sum / 4;
			if (value < 0) {
				bin[2] = 0;
			} else {
				bin[2] = value;
			}
			sum = 0;
		}
	}
	//printArray(bin, 3);
	int v = bin[0]*4 + bin[1]*2 + bin[2]*1;
	printf("Received value = %d\n", v);
}

int connectToServer(int *sock,  struct sockaddr_in *address) {
	int sockfd, connfd;
	struct sockaddr_in servaddr, cli;

	// socket create and varification
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd == -1) {
		printf("socket creation failed...\n");
		return -1;
	}
	bzero(&servaddr, sizeof(servaddr));

	// assign IP, PORT
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = inet_addr(IP);
	servaddr.sin_port = htons(PORT);

	// connect the client socket to server socket
	if (connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) != 0) {
		printf("connection with the server failed...\n");
		return -1;
	}
	*sock = sockfd;
	address = &servaddr;

	return 1;
}

int main(int argc, char const *argv[]) {
	/* code */
	if (argc != 3 ) {
		cout << "Incorrect number of arguments." << endl;
		exit(0);
	}

	int id, value;
	strcpy(IP, argv[1]);
	PORT = atoi(argv[2]);

	int des[3];
	int values[3];

	int i = 0;
	while (cin >> id >> value) {
		//cout << id << value << endl;
		des[i] = id;
		values[i] = value;
		i++;
		if (i >= 3) {
			break;
		}
	}

	int pid1, pid2, pid3;
	int *id1, *id2, *id3;

	pid1 = fork();

	if (pid1 == 0) {
		struct sockaddr_in address1;
		int sock1 = 0, valread;
		struct sockaddr_in serv_addr;
		char buff[80];
		int sentInfo = 0;
		if (connectToServer(&sock1, &address1) == 1) {
			bzero(buff, sizeof(buff));
			char s[3];
			char d[3];
			char v[3];
			sprintf(s, "%d", 1);
			sprintf(d, "%d", des[0]);
			sprintf(v, "%d", values[0]);
			strcpy(buff, s);
			strcat(buff, ":");
			strcat(buff, d);
			strcat(buff, ":");
			strcat(buff, v);

			write(sock1, buff, sizeof(buff));
			printf("Child %s, sending value: %s to child process %s\n", s, v, d);
			sentInfo = 1;
			bzero(buff, sizeof(buff));
			int EM[12];
			int w[4];
			read(sock1, EM, sizeof(EM));
			read(sock1, w, sizeof(w));
			//printf("Received: %s\n", buff);
			decode(EM, w, 1);
			//	break;
		}
		sleep(1);
	} else {
		pid2 = fork();
		if (pid2 == 0) {
			struct sockaddr_in address1;
			int sock1 = 0, valread;
			struct sockaddr_in serv_addr;
			char buff[80];

			if (connectToServer(&sock1, &address1) == 1) {
				bzero(buff, sizeof(buff));
				char s[3];
				char d[3];
				char v[3];
				sprintf(s, "%d", 2);
				sprintf(d, "%d", des[1]);
				sprintf(v, "%d", values[1]);
				strcpy(buff, s);
				strcat(buff, ":");
				strcat(buff, d);
				strcat(buff, ":");
				strcat(buff, v);
				write(sock1, buff, sizeof(buff));
				printf("Child %s, sending value: %s to child process %s\n", s, v, d);
				bzero(buff, sizeof(buff));
				int EM[12];
				int w[4];
				read(sock1, EM, sizeof(EM));
				read(sock1, w, sizeof(w));
				//printf("Received: %s\n", buff);
				//printArray(EM, 12);
				decode(EM, w, 2);
			}
			sleep(1);
		} else {
			pid3 = fork();
			if (pid3 == 0) {
				struct sockaddr_in address1;
				int sock1 = 0, valread;
				struct sockaddr_in serv_addr;
				char buff[80];

				if (connectToServer(&sock1, &address1) == 1) {
					bzero(buff, sizeof(buff));
					char s[3];
					char d[3];
					char v[3];
					sprintf(s, "%d", 3);
					sprintf(d, "%d", des[2]);
					sprintf(v, "%d", values[2]);
					strcpy(buff, s);
					strcat(buff, ":");
					strcat(buff, d);
					strcat(buff, ":");
					strcat(buff, v);
					write(sock1, buff, sizeof(buff));
					printf("Child %s, sending value: %s to child process %s\n", s, v, d);
					bzero(buff, sizeof(buff));
					int EM[12];
					int w[4];
					read(sock1, EM, sizeof(EM));
					read(sock1, w, sizeof(w));
					//printf("Received: %s\n", buff);
					//printArray(EM, 12);
					decode(EM, w, 3);
				}
				sleep(1);
			} else {
				//cout << "Done";
			}
		}
	}

	int stat;
	pid_t cpid1 = waitpid(pid1, &stat, 0);
	pid_t cpid2 = waitpid(pid2, &stat, 0);
	pid_t cpid3 = waitpid(pid3, &stat, 0);




	return 0;
}