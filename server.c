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
#include <time.h>

// Global Variables
char buffer[1000];

FILE *fileQuiz;
char filenameQuiz [256];	// Nombre del archivo del quiz que se va a ejecutar

int port;

int numPantalla = 1;

// Estructura del cliente
struct AcceptedSocket
{
	int id;
	int acceptedSocketFD;
	int error;
	bool acceptedSuccessfully;
};

// Estructura de una pregunta
struct Pregunta
{
	int numPreg;
	char descripcion[1000];
	int tipo;	// Tipo 1 es Verdadero/Falso, 2 es Respuesta Multiple
	int resCorrecta;	// Si fuera una pregunta Verdadero/Falso, 1 es Verdadero y 2 es Falso
	char vecesVF[50];	// Las veces que se ha respondido con Verdad o Falso
	char resMultiples[4][1000];
	int tiempo;
	int puntos;
	char seVaAUsar[5];
	int vecesUsada;
};

struct Pregunta* preguntaAct;

// Clientes conectados
struct AcceptedSocket acceptedSockets[10];
int acceptedSocketsCount = 0;

// Proceso de aceptar un nuevo cliente
struct AcceptedSocket* acceptIncomingConnection(int serverSocketFD, int client);
void clientThread(int serverSocketFD);
void acceptNewClient(int serverSocketFD);

// Pantallas de HTML
char* pantallaEspera();
char* pantallaInicio();
char* pantallaPregVF(int numPreg, char desc[1000]);
char* pantallaPregMul(int numPreg, char desc[1000], char p1[100], char p2[100], char p3[100], char p4[100]);

// Edicion de HTMLs
char* getFromHTML(char buffer[10000], int opcion);
char* insertID(char buffer[10000], int id);

// Ejecucion de un quiz
void iniciarQuiz();
void randomizarPregs(int max);
void comenzandoQuiz();

// Funciones de apoyo
int getCantPreg();
bool isEmpty();
void moverPreg(int num);
void borrarPreg(int numPreg);
void guardarPreg(char* linea);
struct Pregunta* getPreg(int numPreg);	// Busca cual es la linea de la pregunta
struct Pregunta* getFromLine(char linea [1000]);	// Guarda y retorna los datos de la pregunta
char * getFromRM(char linea[1000], bool desc);	// Dada una respuesta multiple, extrae la descripcion o las veces seleccionada

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
	
	clientThread(serverSocket);
	while(true){
		iniciarQuiz();
	}
	
	return 0;
}

// ===============================================

struct AcceptedSocket* acceptIncomingConnection(int serverSocketFD, int client){
	int clientSocketFD = accept(serverSocketFD, NULL, NULL);
	struct AcceptedSocket* acceptedSocket;
	
	if (client == -1){
		//printf("Un nuevo cliente se ha conectado.\n\n");
		acceptedSocket = calloc(1, sizeof(struct AcceptedSocket));
		
		acceptedSocket->id = acceptedSocketsCount;
		
		acceptedSocket->acceptedSocketFD = clientSocketFD;
		acceptedSocket->acceptedSuccessfully = clientSocketFD > 0;
		
		if (!acceptedSocket->acceptedSuccessfully)
			acceptedSocket->error = clientSocketFD;
			
		acceptedSockets[acceptedSocketsCount] = *acceptedSocket;
		acceptedSocketsCount++;
	} else {
		for (int i = 0; i < 30; i++){
			if (acceptedSockets[i].id == client){
				acceptedSocket = &acceptedSockets[i];
				break;
			}
		}
	}
		
	return acceptedSocket;
}

void clientThread(int serverSocketFD){
	pthread_t id;
	pthread_create(&id, NULL, acceptNewClient, serverSocketFD);
}

void acceptNewClient(int serverSocketFD){
	
	char paginaAct[10000];
	char data[10000];
	char buffer[10000];
	struct AcceptedSocket* clientSocket;
	
	while(true)
	{
		switch(numPantalla){
			case 1:
				strcpy(data, pantallaEspera());
				break;
			case 2:
				strcpy(data, pantallaInicio());
				break;
			case 3:
				strcpy(data, pantallaPregVF(preguntaAct->numPreg, preguntaAct->descripcion));
				break;
			case 4:
				strcpy(data, pantallaPregMul(preguntaAct->numPreg, preguntaAct->descripcion, getFromRM(preguntaAct->resMultiples[0], true), getFromRM(preguntaAct->resMultiples[1], true), getFromRM(preguntaAct->resMultiples[2], true), getFromRM(preguntaAct->resMultiples[3], true)));
		}
	
		strcpy(paginaAct, "HTTP/1.1 200 OK\r\n\n");
		
		// Get ID
		//printf("\n\n", getFromHTML(data, 1));
		//strcpy(buffer, getFromHTML(data, 1));
		
		if (strcmp(buffer, "") == 0){	// Nuevo cliente, nuevo ID
			clientSocket = acceptIncomingConnection(serverSocketFD, -1);
		} else {
			clientSocket = acceptIncomingConnection(serverSocketFD, atoi(buffer));
		}
		
		
		// Guardar ID en HTML
		/*
		strcpy(buffer, insertID(data, clientSocket->id));
		bzero(data, sizeof(data));
		strcpy(data, buffer);
		bzero(buffer, sizeof(buffer));
		*/
		
		strcat(paginaAct, data);
		//printf("=== %s ===\n\n", paginaAct);
		send(clientSocket->acceptedSocketFD, paginaAct, sizeof(paginaAct), 0);
		bzero(paginaAct, sizeof(paginaAct));
		bzero(buffer, sizeof(buffer));
		bzero(data, sizeof(data));
		
		recv(clientSocket->acceptedSocketFD, data, sizeof(data), 0);
		//printf("+++ %s +++\n\n", data);
		bzero(data, sizeof(data));
		
		close(clientSocket->acceptedSocketFD);
		bzero(clientSocket, sizeof(clientSocket));
	}
}

// ===============================================

char* pantallaEspera(){
	char* ret = calloc(1, 10000);
	bzero(ret, sizeof* ret);
	ret = "<!DOCTYPE html>\
	\
	<html>\
	    	<head>\
			<meta charset=\"utf-8\">\
			<meta http-equiv=\"refresh\" content=\"1; URL=http://localhost:2300/?ID=%1%\">\
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

char* pantallaInicio(){
	char* ret = calloc(1, 10000);
	bzero(ret, sizeof* ret);
	ret = "<!DOCTYPE html>\
	\
	<html>\
	    	<head>\
			<meta charset=\"utf-8\">\
			<meta http-equiv=\"refresh\" content=\"1; URL=http://localhost:2300/?ID=%1%\">\
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
			<br/>\
			<h2 style=\"color: #2e6c80;\">&nbsp;</h2>\
			<h2 style=\"color: #2e6c80; text-align: center;\">El quiz va a comenzar en 15 segundos.</h2>\
	    	</body>\
	</html>\0";
	return ret;
}

char* pantallaPregVF(int numPreg, char desc[1000]){
	char* ret = calloc(1, 10000);
	bzero(ret, sizeof* ret);
	sprintf(ret, "<!DOCTYPE html>\
	\
	<html>\
	    	<head>\
			<meta charset=\"utf-8\">\
			<meta http-equiv=\"refresh\" content=\"1; URL=http://localhost:2300/?ID=%1%\">\
			<title>QuizSistemas Cliente Web</title>\
	    	</head>\
	    	<body>\
			<h1 style=\"color: #5e9ca0; text-align: center;\">QuizWeb Proyecto de SO</h1>\
			<br/>\
			<p>Pregunta #%d</p>\
			<br/>\
			<p>%s</p>\
			<br/>\
			<br/>\
			<br/>\
			<br/>\
			<form method=\"GET\">\
				<div>\
					<input type=\"radio\" name=\"p%d\" value=\"verdad\"> Verdad\
					<input type=\"radio\" name=\"p%d\" value=\"falso\"> Falso\
				</div>\
	    	</body>\
	</html>\0", numPreg, desc, numPreg, numPreg);
	return ret;
}

char* pantallaPregMul(int numPreg, char desc[1000], char p1[100], char p2[100], char p3[100], char p4[100]){
	char* ret = calloc(1, 10000);
	bzero(ret, sizeof* ret);
	sprintf(ret, "<!DOCTYPE html>\
	\
	<html>\
	    	<head>\
			<meta charset=\"utf-8\">\
			<meta http-equiv=\"refresh\" content=\"1; URL=http://localhost:2300/?ID=%1%\">\
			<title>QuizSistemas Cliente Web</title>\
	    	</head>\
	    	<body>\
			<h1 style=\"color: #5e9ca0; text-align: center;\">QuizWeb Proyecto de SO</h1>\
			<br/>\
			<p>Pregunta #%d</p>\
			<br/>\
			<p>%s</p>\
			<br/>\
			<br/>\
			<br/>\
			<br/>\
			<form method=\"GET\">\
				<div>\
					<input type=\"radio\" name=\"p%d\" value=\"1\"> %s\
					<input type=\"radio\" name=\"p%d\" value=\"2\"> %s\
					<input type=\"radio\" name=\"p%d\" value=\"3\"> %s\
					<input type=\"radio\" name=\"p%d\" value=\"4\"> %s\
				</div>\
	    	</body>\
	</html>\0", numPreg, desc, numPreg, p1, numPreg, p2, numPreg, p3, numPreg, p4);

	return ret;
}

// ===============================================

// %#data% - 1.ID 2.
char* getFromHTML(char buffer[10000], int opcion){
	int res;
	
	char * current = calloc(1, sizeof *current);
	bzero(current, sizeof(current));
	
	bool save = false;
	bool skip = false;	// Skip al numero de la opcion para obtener la data
	
	for (int i = 0; i < 1000; i++){
		
		if (skip){
			skip = false;
			continue;
		}
		
		if (buffer[i] == '%'){
			if (save){
				return current;
			}
			if (atoi(&buffer[i+1]) == opcion){
				save = true;
				skip = true;
				continue;
			}
		}
		
		if (save)
			strncat(current, &buffer[i], 1);
	}
	
	return NULL;	// Error
}

char* insertID(char buffer[10000], int id){
	char* current = calloc(1, sizeof *current);
	bzero(current, sizeof(current));
	
	char temp[5];
	bool desarrollo = false;	// Para saber diferenciar entre % inicial y final
	bool save = false;	// Se guarda la info
	bool skipOp = false;	// Para saltar la opcion
	
	for (int i = 0; i < 10000; i++){
		if (skipOp){
			skipOp = false;
			strncat(current, &buffer[i], 1);
			continue;
		}
		
		if (save){
			save = false;
			sprintf(temp, "%d", id);
			strcat(current, temp);
		}
		
		if (buffer[i] == '%'){
			if (desarrollo){	// % final
				desarrollo = false;
				strncat(current, &buffer[i], 1);
				continue;
			}
			
			if (atoi(&buffer[i+1]) == 1){	// Encontro el lugar donde se guarda la data
				save = true;
				skipOp = true;
			}

			desarrollo = true;
		}
		
		strncat(current, &buffer[i], 1);
	}
	
	return current;
}

// ===============================================

void iniciarQuiz(){
	
	bzero(filenameQuiz, sizeof(filenameQuiz));
	while (true){
		printf("Inserte el nombre de un quiz existente para ejecutar: \n\n");
		scanf("%s", filenameQuiz);
		printf("\n\n");
		strcat(filenameQuiz, ".txt");
		
		fileQuiz = fopen(filenameQuiz, "r");
		if (fileQuiz == NULL){
			printf("El archivo que escribio no existe, revise que escribio el nombre correcto.\n");
			bzero(filenameQuiz, sizeof(filenameQuiz));
			continue;
		}

		fclose(fileQuiz);
		break;
	}
	
	int cantPreg = getCantPreg();
	
	if (cantPreg == 0){
		printf("El quiz seleccionado esta vacio, intentelo con otro quiz.\n\n");
		iniciarQuiz();
		return;
	}
	randomizarPregs(cantPreg);
}

void randomizarPregs(int max){
	srand(time(0));	// Seed
	
	int reiteraciones = (rand() % (30 - 1 + 1)) + 1;	// Cuantas veces se pone una pregunta de ultima
	int preg;
	
	while (reiteraciones > 0){
		preg = (rand() % (max - 1 + 1)) + 1;
		moverPreg(preg);
		reiteraciones--;
	}
	
	comenzandoQuiz();
}

void comenzandoQuiz(){
	numPantalla = 2;
	
	sleep(15);
	
	int leyendoPreg = 1;
	int cantPregs = getCantPreg();
	while (cantPregs > 0){
		preguntaAct = getPreg(leyendoPreg);
		
		if (preguntaAct->tipo == 1){
			numPantalla = 3;
		} else {
			numPantalla = 4;
		}
		
		sleep(preguntaAct->tiempo);
		
		leyendoPreg++;
	}
	
	numPantalla = 1;
	//numPantalla = 5;
}

// ===============================================

int getCantPreg(){
	if (isEmpty())
		return 0;

	size_t len = 0;
	ssize_t read;

	int contador = 0;
	char* line = NULL;
	
	fileQuiz = fopen(filenameQuiz, "r");
	
	while ((read = getline(&line, &len, fileQuiz)) != -1)
		contador++;
		
	fclose(fileQuiz);

	return contador;
}

bool isEmpty(){
	fileQuiz = fopen(filenameQuiz, "r");
	fseek (fileQuiz, 0, SEEK_END);
	if (0 == ftell(fileQuiz))
		return true;
	fclose(fileQuiz);
	return false;
}

void moverPreg(int num){
	size_t len = 0;
	ssize_t read;

	int contador = 1;
	char* line = NULL;
	
	fileQuiz = fopen(filenameQuiz, "r");
	
	while ((read = getline(&line, &len, fileQuiz)) != -1){
		if (contador == num){
			break;
		}
		contador++;
	}
		
	fclose(fileQuiz);
	
	borrarPreg(num);
	guardarPreg(line);
}

void borrarPreg(int numPreg){
	char data [1000];
	bzero(data, sizeof(data));
	
	size_t len = 0;
	ssize_t read;

	int contador = 1;
	char* line = NULL;
	
	fileQuiz = fopen(filenameQuiz, "r");
	
	while ((read = getline(&line, &len, fileQuiz)) != -1){
		if (contador == numPreg){
			contador++;
			continue;
		}
		contador++;
		strcat(data, line);
	}
		
	fclose(fileQuiz);
	
	fileQuiz = fopen(filenameQuiz, "w");
	
	fprintf(fileQuiz, "%s", data);
	fclose(fileQuiz);
}

void guardarPreg(char* linea){
	fileQuiz = fopen(filenameQuiz, "a");
	char guardar [2000] = {};
	char num [10];
	
	fprintf(fileQuiz, "%s", linea);

	fclose(fileQuiz);
}

struct Pregunta* getPreg(int numPreg){
	
	char linea [1000];
	char current [1000];
	int i;
	
	fileQuiz = fopen(filenameQuiz, "r");
	
	bzero(linea, sizeof(linea));
	
	while (fscanf(fileQuiz, " %[^\n]", linea) != EOF){	// Para cada linea
		bzero(current, sizeof(current));
		for (i = 0; i < 1000; i++){		// Para cada caracter
			
			if (linea[i] == '$'){
				if (atoi(current) == numPreg){	// Linea correcta
					return getFromLine(linea);
				}
				break;			// Linea incorrecta, siguiente linea
			}
			
			strncat(current, &linea[i], 1);
			
		}
	}
	
	fclose(fileQuiz);
	return NULL;
}

struct Pregunta* getFromLine(char linea [1000]){
	char current [1000];
	int contador = 0;
	int resMul = 0;
	
	bzero(current, sizeof(current));
	struct Pregunta* pregunta = malloc(sizeof(struct Pregunta));
	
	for (int i = 0; i < 1000; i++){
		
		if (linea[i] == '|'){
			strcpy(pregunta->resMultiples[resMul], current);
			strcat(pregunta->resMultiples[resMul], "|");
			resMul++;
			bzero(current, sizeof(current));
			continue;
		}
		
		if (linea[i] == '$'){
			
			switch(contador){
				case 0: 
					pregunta->numPreg = atoi(current);
					break;
				case 1:
					strcpy(pregunta->descripcion, current);
					break;
				case 2:
					pregunta->tipo = atoi(current);
					break;
				case 3:
					pregunta->resCorrecta = atoi(current);
					break;
				case 4:
					strcpy(pregunta->vecesVF, current);
					break;
				case 5:
					if (resMul == 0){
						strcpy(pregunta->resMultiples[0], "NULL");
						strcpy(pregunta->resMultiples[1], "NULL");
						strcpy(pregunta->resMultiples[2], "NULL");
						strcpy(pregunta->resMultiples[3], "NULL");
					}
					break;
				case 6:
					pregunta->tiempo = atoi(current);
					break;
				case 7:
					pregunta->puntos = atoi(current);
					break;
				case 8:
					strcpy(pregunta->seVaAUsar, current);
					break;
				case 9:
					pregunta->vecesUsada = atoi(current);
					return pregunta;
			}
			
			contador++;
			bzero(current, sizeof(current));
			
			continue;
		}
		
		strncat(current, &linea[i], 1);
		
	}
	
	return NULL;
}

char * getFromRM(char linea[1000], bool desc){
	
	char * current = malloc(sizeof *current);
	bzero(current, sizeof(current));

	for (int i = 0; i < 1000; i++){
		
		if (linea[i] == '|')
			return current;
	
		if (linea[i] == '^'){
			if (desc){
				return current;
			} else {
				bzero(current, sizeof(current));
				continue;
			}
		}
		
		strncat(current, &linea[i], 1);
	
	}
	
	return NULL;
}


