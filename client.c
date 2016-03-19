
#ifdef _WIN32
#include<winsock2.h>
#else
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/select.h>
#include <arpa/inet.h>
#endif

#include<signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define BUFFLEN 64

#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_BLUE    "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN    "\x1b[36m"
#define ANSI_COLOR_RESET   "\x1b[0m"


void error(char* message, int socket) {
	fprintf(stderr, "ERROR:%s\n", message);
#ifdef _WIN32
	closesocket(socket);
	WSACleanup();
#else
	close(socket);
#endif // _WIN32
	exit(1);
}

void printSymbol(char c) {
	if (c == 'X') {
		printf(ANSI_COLOR_RED "X" ANSI_COLOR_RESET);
	}
	else if (c == 'O') {
		printf(ANSI_COLOR_BLUE "O" ANSI_COLOR_RESET);
	}
	else {
		printf("%c", c);
	}
}

void printTable(int moves[][3]) {
#ifdef _WIN32
	system("cls");
#else
	system("clear");
#endif // _WIN32
	char symbols[3][3] = { {'1','2','3'},{'4','5','6'},{'7','8','9'} };
	int i, j;
	for (i = 0; i < 3; i++) {
		for (j = 0; j < 3; j++) {
			switch (moves[i][j]) {
			case 0:
				symbols[i][j] = 'O';
				break;
			case 1:
				symbols[i][j] = 'X';
				break;
			default:
				break;
			}
		}
	}
	printf(" ");
	printSymbol(symbols[0][0]);
	printf(" | ");
	printSymbol(symbols[0][1]);
	printf(" | ");
	printSymbol(symbols[0][2]);
	printf(" \n");
	printf("---+---+---\n");
	printf(" ");
	printSymbol(symbols[1][0]);
	printf(" | ");
	printSymbol(symbols[1][1]);
	printf(" | ");
	printSymbol(symbols[1][2]);
	printf(" \n");
	printf("---+---+---\n");
	printf(" ");
	printSymbol(symbols[2][0]);
	printf(" | ");
	printSymbol(symbols[2][1]);
	printf(" | ");
	printSymbol(symbols[2][2]);
	printf(" \n");
}

int checkWin(int moves[][3]) {
	int i;
	//Check horrizontaly
	for (i = 0; i < 3; i++) {
		if ((moves[i][0] == 1 || moves[i][0] == 0) && moves[i][0] == moves[i][1] &&
			moves[i][2] == moves[i][1] && moves[i][2] == moves[i][0])

			return moves[i][0];
	}

	//Check vertically
	for (i = 0; i < 3; i++) {
		if ((moves[0][i] == 1 || moves[0][i] == 0) && moves[0][i] == moves[1][i] &&
			moves[2][i] == moves[1][i] && moves[2][i] == moves[0][i])

			return moves[i][0];
	}

	//Check first diagonal
	if ((moves[1][1] == 1 || moves[1][1] == 0) && moves[0][0] == moves[1][1] &&
		moves[2][2] == moves[1][1] && moves[2][2] == moves[0][0])
		return moves[1][1];

	//Check second diagonal
	if ((moves[1][1] == 1 || moves[1][1] == 0) && moves[2][0] == moves[1][1] &&
		moves[0][2] == moves[1][1] && moves[2][0] == moves[0][2])
		return moves[1][1];
	//If nobody wins -1 is returned
	return -1;
}

int makeMove(int moves[][3], int move, int isX) {
	switch (move) {
	case 1:
		if (moves[0][0] == -1) {
			moves[0][0] = isX;
			return 1;
		}
		break;
	case 2:
		if (moves[0][1] == -1) {
			moves[0][1] = isX;
			return 1;
		}
		break;
	case 3:
		if (moves[0][2] == -1) {
			moves[0][2] = isX;
			return 1;
		}
		break;
	case 4:
		if (moves[1][0] == -1) {
			moves[1][0] = isX;
			return 1;
		}
		break;
	case 5:
		if (moves[1][1] == -1) {
			moves[1][1] = isX;
			return 1;
		}
		break;
	case 6:
		if (moves[1][2] == -1) {
			moves[1][2] = isX;
			return 1;
		}
		break;
	case 7:
		if (moves[2][0] == -1) {
			moves[2][0] = isX;
			return 1;
		}
		break;
	case 8:
		if (moves[2][1] == -1) {
			moves[2][1] = isX;
			return 1;
		}
		break;
	case 9:
		if (moves[2][2] == -1) {
			moves[2][2] = isX;
			return 1;
		}
		break;
	default:
		return -1;
		break;
	}
	return -1;
}

void endGame(int server, int gameNumber) {
	char buffer[BUFFLEN];
	memset(&buffer, 0, BUFFLEN);
	//End game: GAME {game number} END
	sprintf(buffer, "GAME %d END", gameNumber);
	if (send(server, buffer, strlen(buffer), 0) <= 0) {
		error("Connection lost\n", server);
	}
}

int startGameX(int gameNumber, int server) {
	int moves[3][3] = { {-1,-1,-1},{-1,-1,-1},{-1,-1,-1} };
	int move = -1;
	int numMoves = 0;
	char buffer[BUFFLEN], command[BUFFLEN], argument[BUFFLEN];
	printTable(moves);
	while (1) {
		while (1) {
			printf("Place X (enter cell number): ");
			if (scanf("%d", &move) == 0) {
				fflush(stdin);
				continue;
			}
			if (makeMove(moves, move, 1) == 1) {
				numMoves++;
				memset(&buffer, 0, BUFFLEN);
				//Making move as X player: GAME {game number} MOVEX {cell number}
				sprintf(buffer, "GAME %d MOVEX %d", gameNumber, move);
				if (send(server, buffer, strlen(buffer), 0) <= 0) {
					error("Lost connection with server\n", server);
				};

				memset(&buffer, 0, BUFFLEN);
				if (recv(server, buffer, BUFFLEN, 0) <= 0) {
					error("Lost connection with server\n", server);
				}

				sscanf(buffer, "%s", command);
				if (strcmp(command, "OK") == 0) {
					break;
				}
				else if (strcmp(command, "ERR") == 0) {
					printf("Game ended, move cannot be send\n");
					endGame(server, gameNumber);
					return -1;
				}
				else if (strcmp(command, "END") == 0) {
					printf("Opponent disconnected\n");
					endGame(server, gameNumber);
					return -1;
				}
			}
			else {
				printf("Invalid move. Try again\n");
			}
		}
		printTable(moves);
		switch (checkWin(moves)) {
		case 1:
			printf("You WON the game!!! Congratulation\n");
			endGame(server, gameNumber);
			return 1;
			break;
		case -1:
			if (numMoves == 9) {
				printf("DRAW\n");
				endGame(server, gameNumber);
				return 1;
			}
			break;
		}
		while (1) {
			printf("Waiting for oppenent move...\n");
			if (recv(server, buffer, BUFFLEN, 0) <= 0) {
				error("Lost connection with server\n", server);
			}
			sscanf(buffer, "%s %s", command, argument);
			//Read opponent move: MOVEO {cell number}
			if (strcmp(command, "MOVEO") == 0 && strlen(argument) > 0) {
				move = strtol(argument, (char **)NULL, 10);
				if (makeMove(moves, move, 0) == -1) {
					printf("Error accured. Game ends\n");
					endGame(server, gameNumber);
					return -1;
				}
				else {
					numMoves++;
					break;
				}
			}
			else if (strcmp(command, "END") == 0) {
				printf("Opponent disconnected\n");
				endGame(server, gameNumber);
				return -1;
			}
			else if (strcmp(command, "ERR") == 0) {
				printf("Error accured. Game ends\n");
				endGame(server, gameNumber);
				return -1;
			}
		}
		printTable(moves);
		switch (checkWin(moves)) {
		case 0:
			printf("Opponent WON the game!!!\n");
			endGame(server, gameNumber);
			return 1;
			break;
		case -1:
			if (numMoves == 9) {
				printf("DRAW\n");
				endGame(server, gameNumber);
				return 1;
			}
			break;
		}
	}
}

int startGameO(int gameNumber, int server) {
	int moves[3][3] = { {-1,-1,-1},{-1,-1,-1},{-1,-1,-1} };
	int numMoves = 0;
	int move = -1;
	char buffer[BUFFLEN], command[BUFFLEN], argument[BUFFLEN];
	printTable(moves);
	while (1) {
		while (1) {
			printf("Waiting for oppenent move...\n");
			if (recv(server, buffer, BUFFLEN, 0) <= 0) {
				error("Lost connection with server\n", server);
			}
			sscanf(buffer, "%s %s", command, argument);
			//Read opponent move: MOVEX {cell number}
			if (strcmp(command, "MOVEX") == 0 && strlen(argument) > 0) {
				move = strtol(argument, (char **)NULL, 10);
				if (makeMove(moves, move, 1) == -1) {
					printf("Error accured. Game ends\n");
					endGame(server, gameNumber);
					return -1;
				}
				else {
					numMoves++;
					break;
				}
			}
			else if (strcmp(command, "END") == 0) {
				printf("Opponent disconnected\n");
				endGame(server, gameNumber);
				return -1;
			}
			else if (strcmp(command, "ERR") == 0) {
				printf("Error accured. Game ends\n");
				endGame(server, gameNumber);
				return -1;
			}
		}
		printTable(moves);
		switch (checkWin(moves)) {
		case 1:
			printf("Opponent WON the game!!!\n");
			endGame(server, gameNumber);
			return 1;
			break;
		case -1:
			if (numMoves == 9) {
				printf("DRAW\n");
				endGame(server, gameNumber);
				return 1;
			}
			break;
		}
		while (1) {
			printf("Place O (enter cell number): ");
			if (scanf("%d", &move) == 0) {
				fflush(stdin);
				continue;
			}
			if (makeMove(moves, move, 0) == 1) {
				numMoves++;
				memset(&buffer, 0, BUFFLEN);
				//Make move as O player: GAME {game number} MOVEO {cell number}
				sprintf(buffer, "GAME %d MOVEO %d", gameNumber, move);
				if (send(server, buffer, strlen(buffer), 0) <= 0) {
					error("Lost connection with server\n", server);
				};

				memset(&buffer, 0, BUFFLEN);
				if (recv(server, buffer, BUFFLEN, 0) <= 0) {
					error("Lost connection with server\n", server);
				}
				sscanf(buffer, "%s", command);
				if (strcmp(command, "OK") == 0) {
					break;
				}
				else if (strcmp(command, "END") == 0) {
					printf("Opponent disconnected\n");
					endGame(server, gameNumber);
					return -1;
				}
				else if (strcmp(command, "ERR") == 0) {
					printf("Game ended, move cannot be send\n");
					endGame(server, gameNumber);
					return -1;
				}
			}
			else {
				printf("Invalid move. Try again\n");
			}
		}
		printTable(moves);
		switch (checkWin(moves)) {
		case 0:
			printf("You WON the game!!! Congratulation\n");
			endGame(server, gameNumber);
			return 1;
			break;
		case -1:
			if (numMoves == 9) {
				printf("DRAW\n");
				endGame(server, gameNumber);
				return 1;
			}
			break;
		}
	}
}

int main(int argc, char *argv[]) {
#ifdef _WIN32
	WSADATA data;
#endif
	int choise = 1;
	unsigned int port;
	int s_socket;
	struct sockaddr_in servaddr;
	fd_set readSet;

	char input[BUFFLEN];
	char buffer[BUFFLEN];
	char command[20], argument1[20], argument2[20];

	int gameNumber;
	int isX = 0;

	int i;

	if (argc != 3) {
		fprintf(stderr, "USAGE: %s <ip> <port>\n", argv[0]);
		exit(1);
	}

	port = atoi(argv[2]);

	if ((port < 1) || (port > 65535)) {
		printf("ERROR #1: invalid port specified.\n");
		exit(1);
	}

#ifdef _WIN32
	WSAStartup(MAKEWORD(2, 2), &data);
#endif

	if ((s_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		fprintf(stderr, "ERROR #2: cannot create socket.\n");
		exit(1);
	}

	memset(&servaddr, 0, sizeof(servaddr));
	servaddr.sin_family = AF_INET;

#ifdef _WIN32
	if (inet_addr(argv[1]) <= 0) {
		fprintf(stderr, "ERROR #3: Invalid remote IP address.\n");
		exit(1);
	}
	servaddr.sin_addr.s_addr = inet_addr(argv[1]);
#else
	if (inet_aton(argv[1], &servaddr.sin_addr) <= 0) {
		fprintf(stderr, "ERROR #3: Invalid remote IP address.\n");
		exit(1);
	}
#endif // _WIN32

	servaddr.sin_port = htons(port); 

	if (connect(s_socket, (struct sockaddr*)&servaddr, sizeof(servaddr)) < 0) {
		fprintf(stderr, "ERROR #4: error in connect().\n");
		exit(1);
	}

	//login in with name: LOGIN {name}
	do {
		printf("Enter your name(max 18 letters):");
		memset(&input, 0, BUFFLEN);
		memset(&buffer, 0, BUFFLEN);
		fgets(input, BUFFLEN, stdin);
		if (strlen(input) > 19) {
			printf("Entered name is too long, try again\n");
		}
		else {
			sprintf(buffer, "%s %s", "LOGIN", input);
			if (send(s_socket, buffer, strlen(buffer), 0) <= 0) {
				error("Lost connection with server\n", s_socket);
			}

			memset(&buffer, 0, BUFFLEN);
			if (recv(s_socket, buffer, BUFFLEN, 0) <= 0) {
				error("Lost connection with server\n", s_socket);
			}
			if (strcmp(buffer, "OK") == 0 || strcmp(buffer, "ERR") == 0) {
			}
			else {
				printf("BUFFER:%s Compare%d", buffer, strcmp(buffer, "OK"));
				error("Bad server response", s_socket);
			}
		}
	} while (strcmp(buffer, "OK") != 0);

	printf("You have successfully login as %s\n", input);

	while (choise != 0) {
		printf("***MENIU***\n1-Get list of players waiting to play\n2-Wait for another player\n0-Exit\n");
		if (scanf("%d", &choise) == 0) {
			fflush(stdin);
			continue;
		}
		switch (choise) {
			//Get list of waiting players: LIST
		case 1:
			memset(&buffer, 0, BUFFLEN);
			strcpy(buffer, "LIST");
			if (send(s_socket, buffer, strlen(buffer), 0) < 0) {
				error("Lost connection with server\n", s_socket);
			};
			memset(&buffer, 0, BUFFLEN);
			int recvLength = recv(s_socket, buffer, BUFFLEN, 0);
			if (recvLength <= 0) {
				error("Lost connection with server\n", s_socket);
			}
			else {
				//If there aren't players who are waiting NOLIST is returned from server
				if (strcmp(buffer, "NOLIST") != 0 && recvLength > 0) {
					printf("%s", buffer);
					printf("Do you want to offer start a game?\n1-YES\n0-NO\n");
					if (scanf("%d", &choise) == 0) {
						fflush(stdin);
						choise = -1;
					}
					switch (choise) {
						//Start game with chosen opponent: START {opponent number from list}
					case 1:
						memset(&buffer, 0, BUFFLEN);
						printf("Enter player number:");
						if (scanf("%d", &choise) == 0) {
							fflush(stdin);
							choise = -1;
						}
						sprintf(buffer, "%s %d", "START", choise);
						if (send(s_socket, buffer, strlen(buffer), 0) < 0) {
							error("Lost connection with server\n", s_socket);
						};

						memset(&buffer, 0, BUFFLEN);
						if (recv(s_socket, buffer, BUFFLEN, 0) <= 0) {
							error("Lost connection with server\n", s_socket);
						}

						sscanf(buffer, "%s %s %s", command, argument1, argument2);
						if (strcmp(command, "BEGGIN") == 0 && strlen(argument1) > 0 && strlen(argument2) > 0) {
							gameNumber = strtol(argument1, (char **)NULL, 10);
							if (strcmp(argument2, "X") == 0) {
								startGameX(gameNumber, s_socket);
								isX = -1;
								gameNumber = -1;
							}
							else if (strcmp(argument2, "O") == 0) {
								startGameO(gameNumber, s_socket);
								isX = -1;
								gameNumber = -1;
							}
							else {
								printf("Try again\n");
							}
						}
						else {
							printf("Try again\n");
						}
						choise = 1;
						break;
					case 0:
					default:
						choise = 1;
						break;
					}
				}
				else {
					printf("No waiting players\n");
				}
			}
			break;
			//Wait for suggestion playing: WAIT
		case 2:
			memset(&buffer, 0, BUFFLEN);
			strcpy(buffer, "WAIT");
			send(s_socket, buffer, strlen(buffer), 0);
			memset(&buffer, 0, BUFFLEN);
			if (recv(s_socket, buffer, BUFFLEN, 0) <= 0) {
				error("Lost connection with server\n", s_socket);
			}
			else if (strcmp("OK", buffer) == 0) {

				printf("Started waiting for opponent.");
				gameNumber = -1;
				memset(&buffer, 0, BUFFLEN);
				FD_ZERO(&readSet);
				FD_SET(s_socket, &readSet);

				select(s_socket + 1, &readSet, NULL, NULL, NULL);

				if (FD_ISSET(s_socket, &readSet)) {
					if (recv(s_socket, buffer, BUFFLEN, 0) <= 0) {
						error("Lost connection with server\n", s_socket);
					}
					sscanf(buffer, "%s %s %s", command, argument1, argument2);
					if (strcmp(command, "BEGGIN") == 0 && strlen(argument1) > 0 && strlen(argument2) > 0) {
						gameNumber = strtol(argument1, (char **)NULL, 10);
						if (strcmp(argument2, "X") == 0 && gameNumber != -1) {
							printf("PLAYER FOUND\n");
							startGameX(gameNumber, s_socket);
							isX = -1;
							gameNumber = -1;
						}
						else if (strcmp(argument2, "O") == 0 && gameNumber != -1) {
							printf("PLAYER FOUND\n");
							startGameO(gameNumber, s_socket);
							isX = -1;
							gameNumber = -1;
						}
						else {
							printf("Error accured. Try again\n");
						}
					}
					else {
						printf("Try again\n");
					}
				}
			}
			else {
				printf("Try again\n");
			}
			break;
		case 0:
			break;
		default:
			choise = 1;
		}
	}

#ifdef _WIN32
	closesocket(s_socket);
	WSACleanup();
#else
	close(s_socket);
#endif // _WIN32

	return 0;
}
