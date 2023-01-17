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
char buffer[1000];

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
<<<<<<< HEAD
=======
void clientThread(int serverSocketFD);
>>>>>>> parent of 3427b5f (Update server.c)
void acceptNewClient(int serverSocketFD);
void quizThread();

<<<<<<< HEAD
=======
char* pruebHTML();
int getID(char linea[1000]);
char* insertID(char linea[1000]);

>>>>>>> parent of 3427b5f (Update server.c)
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
	server_addr.sin_port = htons(port);
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
	
<<<<<<< HEAD
	struct AcceptedSocket* acceptedSocket = malloc(sizeof(struct AcceptedSocket));
=======
	struct AcceptedSocket* acceptedSocket = calloc(1, 5 + sizeof(struct AcceptedSocket));
	
>>>>>>> parent of 3427b5f (Update server.c)
	acceptedSocket->acceptedSocketFD = clientSocketFD;
	acceptedSocket->acceptedSuccessfully = clientSocketFD > 0;
	
	printf("Un cliente se ha conectado.\n\n");
	
	
	if (!acceptedSocket->acceptedSuccessfully)
		acceptedSocket->error = clientSocketFD;
		
	return acceptedSocket;
}

void acceptNewClient(int serverSocketFD){
	
<<<<<<< HEAD
	filePEspera = fopen("pespera.html", "r");

	char paginaEspera[10000] = "HTTP/1.1 200 OK\r\n\n";
	
	while(true)
	{
		struct AcceptedSocket* clientSocket = acceptIncomingConnection(serverSocketFD);
		acceptedSockets[acceptedSocketsCount++] = *clientSocket;
		while (fgets(buffer, sizeof(buffer), filePEspera) != NULL){
			strcat(paginaEspera, buffer);
			bzero(buffer, sizeof(buffer));
		}
		send(clientSocket->acceptedSocketFD, paginaEspera, 10000, 0);
		//printf("=== %s ===\n\n", paginaEspera);
		close(clientSocket->acceptedSocketFD);
	}
}

void quizThread(){
	pthread_t id;
	pthread_create(&id, NULL, iniciarQuiz, NULL);
=======
	char paginaAct[10000];
	int countCh = 0;
	char prueba[10000];
	strcpy(prueba, pruebHTML());
	printf("=== %s ===\n\n", prueba);
	
	while(true)
	{
		strcpy(paginaAct, "HTTP/1.1 200 OK\r\n\n");
		//filePEspera = fopen("pespera.html", "r");
		strcat(paginaAct, prueba);

		struct AcceptedSocket* clientSocket = acceptIncomingConnection(serverSocketFD);

		//acceptedSockets[acceptedSocketsCount] = *clientSocket;
		
		int i = 0;	// Num de linea
		int id;
		countCh = 0;
		
		
		//printf("=== %s ===\n\n", paginaAct);
		//printf("%d\n\n", countCh);
		send(clientSocket->acceptedSocketFD, paginaAct, sizeof(paginaAct), 0);
		bzero(paginaAct, sizeof(paginaAct));
		
		//recv(clientSocket->acceptedSocketFD, paginaAct, 10000, 0);
		//printf("--- %s ---\n\n", paginaAct);
		bzero(paginaAct, sizeof(paginaAct));
		//bzero(buffer, sizeof(buffer));
		
		close(clientSocket->acceptedSocketFD);
		//fclose(filePEspera);
		bzero(clientSocket, sizeof(clientSocket));
		//free(clientSocket);
		//clientSocket = NULL;
	}
}

char* pruebHTML(){
	char* ret = calloc(1, 10000);
	ret = "<!DOCTYPE html>\
\
<html>\
    <head>\
        <meta charset=\"utf-8\">\
        <meta http-equiv=\"refresh\" content=\"5; URL=http://localhost:2300/?ID=%\">\
        <title>QuizSistemas Cliente Web</title>\
    </head>\
    <body>\
        <h1 style=\"color: #5e9ca0; text-align: center;\">QuizWeb Proyecto de SO</h1>\
        <br/>\
        <p style=\"text-align: center;\">Proyecto #1</p>\
        <p style=\"text-align: center;\">Principios de Sistemas Operativos</p>\
        <p style=\"text-align: center;\">Profesor Esteban Arias M&eacute;ndez</p>\
        <p style=\"text-align: center;\">&nbsp;</p>\
        <p style=\"text-align: center;\">Luis Chavarr&iacute;a Enr&iacute;quez - 2017100034</p>\
        <p style=\"text-align: center;\">David Su&aacute;rez Acosta - 2020038304</p>\
        <br/>\
        <h2 style=\"color: #2e6c80;\">&nbsp;</h2>\
        <h2 style=\"color: #2e6c80; text-align: center;\">Esperando a que el quiz comience, por favor espere...</h2>\
    </body>\
</html>\0";
	return ret;
}

int getID(char linea[1000]){
	int res;
	
	char * current = calloc(1, 5 + sizeof *current);
	bzero(current, sizeof(current));
	
	int skip = 0;
	bool id = false;
	
	for (int i = 0; i < 1000; i++){
		if (linea[i] == '%')
			return -1;
		if (linea[i] == '?'){
			skip = 4;
			id = true;
		}
		
		if (skip != 0){
			skip--;
			continue;
		}
		
		if (id){
			if (linea[i] == '"'){
				break;
			}
			strncat(current, &linea[i], 1);
		}
	
	}
	
	if (strcmp(current, "") == 0)
		return -1;
	
	res = atoi(current);
	return res;
}

char* insertID(char linea[1000]){

	char* current = calloc(1, 5 + sizeof *current);
	bzero(current, sizeof(current));
	
	int skip = 0;
	bool id = false;
	
	for (int i = 0; i < 1000; i++){
		if (linea[i] == '?'){
			skip = 5;
			id = true;
		}
		
		if (skip != 0){
			skip--;
		}
		
		if (id && skip == 0){
			id = false;
			char temp[10];
			sprintf(temp, "%d", acceptedSocketsCount);
			acceptedSocketsCount++;
			strcat(current, temp);
			continue;
		}
		strncat(current, &linea[i], 1);
	}
	
	return current;
>>>>>>> parent of 3427b5f (Update server.c)
}

// ===============================================

void iniciarQuiz(){
	
<<<<<<< HEAD
=======
	bzero(filenameQuiz, sizeof(filenameQuiz));
	while (true){
		printf("Inserte el nombre de un quiz existente para ejecutar: \n\n");
		scanf("%s", filenameQuiz);
		printf("\n\n");
		strcat(filenameQuiz, ".txt");
		
		fileQuiz = fopen(filenameQuiz, "r");
		if (fileQuiz == NULL){
			printf("El archivo que escribio no existe, revise que escribio el nombre correcto.");
			bzero(filenameQuiz, sizeof(filenameQuiz));
			continue;
		}

		bzero(filenameQuiz, sizeof(filenameQuiz));
		break;
	}
	
	
>>>>>>> parent of 3427b5f (Update server.c)
}




