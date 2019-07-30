#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <unistd.h>
#include <math.h>
#include <time.h>
#include <pthread.h>
#include <semaphore.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <cstdlib>
#include <cstring>
#include <sys/wait.h>
#define bzero(b,len) (memset((b), '\0', (len)), (void) 0)


using namespace std;

int port;

int w[4][4] = {{ -1, -1, -1, -1}, { -1, 1, -1, 1},
	{ -1, -1, 1, 1}, { -1, 1, 1, -1}
};

int EM1[12];
int EM2[12];
int EM3[12];
int EM[12];

int em1Done = 0;
int em2Done = 0;
int em3Done = 0;
int emDone = 0;

int dataReceived = 0;

void initializeServer(int *serverSocket,
                      struct sockaddr_in  *serverAddress) {
	int opt = 1;

	// if ((*serverSocket = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
	//       perror("socket failed");
	//       exit(EXIT_FAILURE);
	// }
	struct sockaddr_in  address;
	int sockId;

	sockId = socket(AF_INET, SOCK_STREAM, 0);
	if (sockId == -1) {
		perror("socket creation failed...\n");
		exit(EXIT_FAILURE);
	}

	bzero(&address, sizeof(address));

	address.sin_family = AF_INET;
	address.sin_addr.s_addr = htonl(INADDR_ANY);
	address.sin_port = htons( port );

	if ((bind(sockId, (struct sockaddr *)&address, sizeof(address))) != 0) {
		printf("socket bind failed...\n");
		exit(0);
	}

	if ((listen(sockId, 20)) != 0) {
		printf("Listen failed...\n");
		exit(0);
	}

	//send(new_socket , hello , strlen(hello) , 0 );
	//printf("Hello message sent\n");
	*serverSocket = sockId;
	serverAddress = &address;
}

void *handleIncomingRequests(void *e) {
	int socId;
	socklen_t len;
	struct sockaddr_in address, cli1, cli2, cli3;
	// Initialize the server
	initializeServer(&socId, &address);
	// Wait for clients now

	//while (1) {
	int n_socket[3], data;

	len = sizeof(cli1);
	n_socket[0] = accept(socId, (struct sockaddr *)&cli1, &len);
	if (n_socket[0] < 0) {
		printf("server acccept failed...\n");
		exit(0);
	}

	len = sizeof(cli2);
	n_socket[1] = accept(socId, (struct sockaddr *)&cli2, &len);
	if (n_socket[1] < 0) {
		printf("server acccept failed...\n");
		exit(0);
	}

	len = sizeof(cli3);
	n_socket[2] = accept(socId, (struct sockaddr *)&cli3, &len);
	if (n_socket[2] < 0) {
		printf("server acccept failed...\n");
		exit(0);
	}
	int clients[3];

	for (int k = 0; k < 3; k++) {
		char buff[80];
		read( n_socket[k] , buff , sizeof(buff));
		char s[3];
		char d[3];
		char v[3];
		int count = 0, sIndex = 0, dIndex = 0, vIndex = 0;
		for (int i = 0; i < strlen(buff); i++) {
			if (buff[i] != ':') {
				if (count == 0) {
					s[sIndex++] = buff[i];
				}
				if (count == 1) {
					d[dIndex++] = buff[i];
				}
				if (count == 2) {
					v[vIndex++] = buff[i];
				}
			} else {
				count++;
			}
		}

		s[sIndex] = '\0';
		d[dIndex] = '\0';
		v[vIndex] = '\0';
		printf("%s %s: Value = %s, Destination = %s\n", "Here is the message from child", s, d, v);

		int id = atoi(v);
		int bin[3];
		int tmp = id;
		int idx = 2;
		while (tmp > 0 && idx >= 0) {
			int t = tmp % 2;
			if (t == 0) {
				bin[idx] = -1;
			} else {
				bin[idx] = 1;
			}
			idx--;
			tmp /= 2;
		}
		while (idx >= 0) {
			bin[idx--] = -1;
		}

		int wIndex = atoi(s);
		int c = 0;
		int dIdx = atoi(d);
		clients[dIdx-1] = atoi(s);
		if (wIndex == 1) {
			c = 0;
			for (int i = 0; i < 12; i++) {
				if (i <= 3) {
					c = 0;
				} else if (i >= 4 && i <= 7) {
					c = 1;
				} else {
					c = 2;
				}
				EM1[i] = w[wIndex][ i % 4 ] * bin[c];
			}
			em1Done = 1;
		}

		if (wIndex == 2) {
			c = 0;
			for (int i = 0; i < 12; i++) {
				if (i <= 3) {
					c = 0;
				} else if (i >= 4 && i <= 7) {
					c = 1;
				} else {
					c = 2;
				}
				EM2[i] = w[wIndex][ i % 4 ] * bin[c];
			}
			em2Done = 1;
		}

		if (wIndex == 3) {
			c = 0;
			for (int i = 0; i < 12; i++) {
				if (i <= 3) {
					c = 0;
				} else if (i >= 4 && i <= 7) {
					c = 1;
				} else {
					c = 2;
				}
				EM3[i] = w[wIndex][ i % 4 ] * bin[c];
			}
			em3Done = 1;
		}
	}

	for (int k = 0; k < 3; k++) {
		if (em1Done == 1 && em2Done == 1 && em3Done == 1) {
			for (int i = 0; i < 12; i++) {
				EM[i] = EM1[i] + EM2[i] + EM3[i];
			}
			write(n_socket[k], EM, sizeof(EM));
			write(n_socket[k], w[clients[k]], sizeof(w[clients[k]]));
			sleep(1);
		}
	}
}

int main(int argc, char const * argv[])
{
	if (argc != 2 ) {
		cout << "Incorrect number of arguments." << endl;
		exit(0);
	}
	int id, value;
	port = atoi(argv[1]);

	pthread_t thread;
	pthread_create(&thread, NULL, handleIncomingRequests, NULL);
	pthread_join(thread, NULL);

	return 0;
}