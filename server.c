
#ifdef _WIN32
#include<winsock2.h>
#else
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/select.h>
#endif
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <time.h>

#define BUFFLEN 64
#define MAXCLIENTS 10
#define MAXGAMES 5

typedef struct {
	int userSocket;
	int isWaiting;
	char userName[20];
	struct in_addr sin_addr;
} user;

typedef struct {
	int isStarted;
	user* playerX;
	user* playerO;
}game;

int findemptyuser(user clients[]) {
	int i;
	for (i = 0; i < MAXCLIENTS; i++) {
		if (clients[i].userSocket == -1) {
			return i;
		}
	}
	return -1;
}

int findEmptyGame(game games[]) {
	int i;
	for (i = 0; i < MAXGAMES; i++) {
		if (games[i].isStarted == 0) {
			games[i].playerO = NULL;
			games[i].playerX = NULL;
			return i;
		}
		else if (games[i].playerO->userSocket == -1 || games[i].playerX->userSocket == -1) {
			return i;
		}
	}
	return -1;
}

int ifX() {
	int random = rand() % 100;
	if (random < 50) {
		return 1;
	}
	else {
		return 0;
	}
}

int sendMessage(char message[], user *client) {
	int sendLenght = send(client->userSocket, message, strlen(message), 0);
	if (sendLenght <= 0) {
#ifdef _WIN32
		closesocket(client->userSocket);
#else
		close(client->userSocket);
#endif // _WIN32
		client->userSocket = -1;
		strcpy(client->userName, "NoName");
		client->isWaiting = 0;
		return -1;
	}
	return 1;
}

void gameCommand(char* gameNumber, char* command, char* argument, game* games, user* client) {
	int gameNum = strtol(gameNumber, (char **)NULL, 10);
	char buffer[BUFFLEN];
	game current = games[gameNum];
	memset(&buffer, 0, BUFFLEN);
	if (current.isStarted = 1 && strcmp(command, "MOVEX") == 0 && strlen(argument) > 0
		&& client->userSocket == (current.playerX)->userSocket) {
		int moveNumber = strtol(argument, (char **)NULL, 10);
		sprintf(buffer, "MOVEX %d", moveNumber);
		if (sendMessage(buffer, (current.playerO)) > 0) {
			memset(&buffer, 0, BUFFLEN);
			strcpy(buffer, "OK");
			sendMessage(buffer, client);
		}
		else {
			memset(&buffer, 0, BUFFLEN);
			strcpy(buffer, "ERR");
			sendMessage(buffer, client);
		}
	}
	else if (current.isStarted = 1 && strcmp(command, "MOVEO") == 0 && strlen(argument) > 0 && client->userSocket == (current.playerO)->userSocket) {
		int moveNumber = strtol(argument, (char **)NULL, 10);
		sprintf(buffer, "MOVEO %d", moveNumber);;
		if (sendMessage(buffer, (current.playerX)) > 0) {
			memset(&buffer, 0, BUFFLEN);
			strcpy(buffer, "OK");
			sendMessage(buffer, client);
		}
		else {
			memset(&buffer, 0, BUFFLEN);
			strcpy(buffer, "ERR");
			sendMessage(buffer, client);
		}
	}
	else if (strcmp(command, "END") == 0) {
		current.isStarted = 0;
		if (current.playerO->userSocket == client->userSocket) {
			current.playerO = NULL;
		}
		else if (current.playerX->userSocket == client->userSocket) {
			current.playerX = NULL;
		}
	}
	else {
		strcpy(buffer, "ERR");
		sendMessage(buffer, client);
	}
}


void proccessCommand(char commandText[], user* client, user* clients, game* games) {
	int i = 0, count = 0;
	char command[BUFFLEN], argument1[BUFFLEN], argument2[BUFFLEN], argument3[BUFFLEN], buffer[BUFFLEN];
	printf("Procced command:%s", commandText);
	sscanf(commandText, "%s %s %s %s", command, argument1, argument2, argument3);
	memset(&buffer, 0, BUFFLEN);
	if (strcmp(command, "LIST") == 0) {
		for (i = 0; i < MAXCLIENTS; i++) {
			if (clients[i].userSocket != client->userSocket && clients[i].userSocket != -1 && clients[i].isWaiting == 1) {
				count++;
				char iNumber[2];
				sprintf(iNumber, "%d", i);
				strcat(buffer, iNumber);
				strcat(buffer, "-");
				strcat(buffer, clients[i].userName);
				strcat(buffer, "\n");
			}
		}
		if (count != 0) {
			sendMessage(buffer, client);
		}
		else {
			memset(&buffer, 0, BUFFLEN);
			strcpy(buffer, "NOLIST");
			sendMessage(buffer, client);
		}
	}
	else if (strcmp(command, "LOGIN") == 0 && strlen(argument1)>0 && strlen(argument1)<19) {
		strcpy(buffer, "OK");
		strcpy(client->userName, argument1);
		sendMessage(buffer, client);
	}
	else if (strcmp(command, "WAIT") == 0) {
		strcpy(buffer, "OK");
		client->isWaiting = 1;
		sendMessage(buffer, client);
	}
	//Starting game with another player who is waiting
	else if (strcmp(command, "START") == 0 && strlen(argument1) > 0) {
		int isFound = 0;
		int number = strtol(argument1, (char **)NULL, 10);
		if (clients[number].isWaiting == 1 && clients[number].userSocket != -1) {
			int gameNumber = findEmptyGame(games);
			if (gameNumber != -1) {
				games[gameNumber].isStarted = 1;
				switch (ifX()) {
				case 1:
					games[gameNumber].playerX = client;
					games[gameNumber].playerO = &clients[number];

					memset(&buffer, 0, BUFFLEN);
					sprintf(buffer, "BEGGIN %d X", gameNumber);
					sendMessage(buffer, client);

					memset(&buffer, 0, BUFFLEN);
					sprintf(buffer, "BEGGIN %d O", gameNumber);
					sendMessage(buffer, &clients[number]);


					break;
				case 0:
					games[gameNumber].playerO = client;
					games[gameNumber].playerX = &clients[number];

					memset(&buffer, 0, BUFFLEN);
					sprintf(buffer, "BEGGIN %d X", gameNumber);
					sendMessage(buffer, &clients[number]);

					memset(&buffer, 0, BUFFLEN);
					sprintf(buffer, "BEGGIN %d O", gameNumber);
					sendMessage(buffer, client);

					break;
				}
				clients[number].isWaiting = 0;
				client->isWaiting = 0;
			}
			else {
				strcpy(buffer, "ERR");
				sendMessage(buffer, client);
			}
		}
		else {
			strcpy(buffer, "ERR");
			sendMessage(buffer, client);
		}
	}
	else if (strcmp(command, "GAME") == 0 && strlen(argument1) > 0 && strlen(argument2) > 0) {
		gameCommand(argument1, argument2, argument3, games, client);
	}
	else {
		strcpy(buffer, "ERR");
		sendMessage(buffer, client);
	}
}
void closeWithEnd(int client, game games[]) {
	int i;
	char buffer[BUFFLEN];
	for (i = 0; i < MAXGAMES; i++) {
		if (games[i].playerO != NULL) {
			if ((games[i].playerO)->userSocket == client && games[i].isStarted == 1 && (games[i].playerX)->userSocket != -1) {
				memset(&buffer, 0, BUFFLEN);
				strcpy(buffer, "END");
				sendMessage(buffer, games[i].playerX);
				games[i].isStarted = 0;
				break;
			}
		}
		if (games[i].playerX != NULL) {
			if ((games[i].playerX)->userSocket == client && games[i].isStarted == 1 && (games[i].playerO)->userSocket != -1) {
				memset(&buffer, 0, BUFFLEN);
				strcpy(buffer, "END");
				sendMessage(buffer, games[i].playerO);
				games[i].isStarted = 0;
				break;
			}
		}
	}
#ifdef _WIN32
	closesocket(client);
#else
	close(client);
#endif // _WIN32
}



int main(int argc, char *argv[]) {
#ifdef _WIN32
	WSADATA data;
#endif
	unsigned int port;
	unsigned int clientaddrlen;
	int listenSocket;
	struct sockaddr_in servaddr;
	struct sockaddr_in clientaddr;

	user clients[MAXCLIENTS];
	game games[MAXGAMES];

	fd_set read_set;
	int maxfd = 0;

	int i;
	char buffer[BUFFLEN];

	if (argc != 2) {
		fprintf(stderr, "USAGE: %s <port>\n", argv[0]);
		return -1;
	}

	port = atoi(argv[1]);
	if ((port < 1) || (port > 65535)) {
		fprintf(stderr, "ERROR #1: invalid port specified.\n");
		return -1;
	}

#ifdef _WIN32
	WSAStartup(MAKEWORD(2, 2), &data);
#endif

	if ((listenSocket = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		fprintf(stderr, "ERROR #2: cannot create listening socket.\n");
		return -1;
	}

	memset(&servaddr, 0, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port = htons(port);

	if (bind(listenSocket, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
		fprintf(stderr, "ERROR #3: bind listening socket.\n");
		return -1;
	}

	if (listen(listenSocket, 5) < 0) {
		fprintf(stderr, "ERROR #4: error in listen().\n");
		return -1;
	}

	for (i = 0; i < MAXCLIENTS; i++) {
		clients[i].userSocket = -1;
		strcpy(clients[i].userName, "NoName");
		clients[i].isWaiting = 0;
	}

	for (i = 0; i < MAXGAMES; i++) {
		games[i].isStarted = 0;
		games[i].playerO = NULL;
		games[i].playerX = NULL;
	}

	srand(time(NULL));

	for (;;) {
		printf("\nConnected users:\n");
		for (i = 0; i < MAXCLIENTS; i++) {
			if (clients[i].userSocket != -1) {
				printf("Client %d Name %s IP address:%s handler:%d WAIT:%d\n", i, clients[i].userName, inet_ntoa(clients[i].sin_addr), clients[i].userSocket, clients[i].isWaiting);
			}
		}

		FD_ZERO(&read_set);
		for (i = 0; i < MAXCLIENTS; i++) {
			if (clients[i].userSocket != -1) {
				FD_SET(clients[i].userSocket, &read_set);
				if (clients[i].userSocket > maxfd) {
					maxfd = clients[i].userSocket;
				}
			}
		}

		FD_SET(listenSocket, &read_set);
		if (listenSocket > maxfd) {
			maxfd = listenSocket;
		}

		if (select(maxfd + 1, &read_set, NULL, NULL, NULL) < 0) {
			fprintf(stderr, "ERROR #5: error in select().\n");
			return -1;
		}

		if (FD_ISSET(listenSocket, &read_set)) {
			int client_id = findemptyuser(clients);
			if (client_id != -1) {
				clientaddrlen = sizeof(clientaddr);
				memset(&clientaddr, 0, clientaddrlen);
				clients[client_id].userSocket = accept(listenSocket,
					(struct sockaddr*)&clientaddr, &clientaddrlen);
				clients[client_id].sin_addr = clientaddr.sin_addr;
			}
		}

		for (i = 0; i < MAXCLIENTS; i++) {
			if (clients[i].userSocket != -1) {
				if (FD_ISSET(clients[i].userSocket, &read_set)) { 
					memset(&buffer, 0, BUFFLEN);
					//check if client is connected
					if (recv(clients[i].userSocket, buffer, BUFFLEN, 0) <= 0) {
						closeWithEnd(clients[i].userSocket, games);
						clients[i].userSocket = -1;
						clients[i].isWaiting = 0;
					}
					else {
						proccessCommand(buffer, &clients[i], clients, games);
					}
				}
			}
		}
	}

	return 0;
}
