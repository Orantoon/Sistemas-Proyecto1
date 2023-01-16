#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <errno.h>

// Global Variables
char buffer[1024];

FILE *fileQuiz;
char filenameQuiz [256];	// Nombre del archivo del quiz

int port;

FILE *filePEspera;
FILE *filePVerFal;
FILE *filePResMul;

// Estructura del cliente
struct AcceptedSocket
{
	int acceptedSocketFD;
	int error;
	bool acceptedSuccessfully;
};

// Clientes conectados
struct AcceptedSocket acceptedSockets[30];
int acceptedSocketsCount = 0;

// Proceso de aceptar un nuevo cliente
struct AcceptedSocket* acceptIncomingConnection(int serverSocketFD);
void acceptNewClient(int serverSocketFD);
void quizThread();

// Quiz
void iniciarQuiz();

// recv(new_socket, buffer, 1024, 0);			RECEIVE
// send(new_socket, buffer, strlen(buffer), 0);	SEND

int main(){
  
	printf("Inserte el puerto que desea usar: ");
	scanf("%d", &port);
	printf("\n\n");
	
	// ---------- Configuracion de Sockets -----------
	
	// create a socket
	int serverSocket = socket(AF_INET, SOCK_STREAM, 0);

	struct sockaddr_in server_addr;
	socklen_t addr_size;

	if (serverSocket < 0){
		perror("Error in the connection.\n\n");
		exit(1);
	}
	printf("Server Socket created successfully!\n\n");

	memset(&server_addr, '\0', sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = port;
	server_addr.sin_addr.s_addr = INADDR_ANY;
	
	int yes = 1;
	if (setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) == -1){
		perror("setsockopt");
		exit(1);
	}	

	// bind the socket to our specified IP and port
	int ret = bind(serverSocket, (struct sockaddr*)&server_addr, sizeof(server_addr));
	if (ret < 0){
		perror("Error in binding.\n\n");
		exit(1);
	}
	printf("Binded to port %d.\n\n", port);

	// listen to connections
	if (listen(serverSocket, 10) == 0){
		printf("Listening...\n\n");
	} else {
		printf("Error in binding.\n\n");
	}

	// -------------------------------------------
	
	quizThread();
	acceptNewClient(serverSocket);
	
	printf("Nos vemos!\n");
	
	shutdown(serverSocket, SHUT_RDWR);
	
	return 0;
}

// ===============================================

struct AcceptedSocket* acceptIncomingConnection(int serverSocketFD){
	int clientSocketFD = accept(serverSocketFD, NULL, NULL);
	
	struct AcceptedSocket* acceptedSocket = malloc(sizeof(struct AcceptedSocket));
	acceptedSocket->acceptedSocketFD = clientSocketFD;
	acceptedSocket->acceptedSuccessfully = clientSocketFD > 0;
	
	printf("Un cliente se ha conectado.\n\n");
	
	
	if (!acceptedSocket->acceptedSuccessfully)
		acceptedSocket->error = clientSocketFD;
		
	return acceptedSocket;
}

void acceptNewClient(int serverSocketFD){
	
	filePEspera = fopen("pespera.html", "r");
	fgets(buffer, sizeof(buffer), filePEspera);

	char paginaEspera[2000] = "HTTP/1.1 200 OK\r\n\n";
	strcat(paginaEspera, buffer);
	bzero(buffer, sizeof(buffer));
	
	while(true)
	{
		struct AcceptedSocket* clientSocket = acceptIncomingConnection(serverSocketFD);
		acceptedSockets[acceptedSocketsCount++] = *clientSocket;
		send(clientSocket->acceptedSocketFD, paginaEspera, 2000, 0);
		close(clientSocket->acceptedSocketFD);
	}
}

void quizThread(){
	pthread_t id;
	pthread_create(&id, NULL, iniciarQuiz, NULL);
}

// ===============================================

void iniciarQuiz(){
	
}




