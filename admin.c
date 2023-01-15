#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <sys/types.h>
#include <string.h>
#include <unistd.h>

// Global Variables
FILE *file;

char archivo [256];	// Nombre del archivo del quiz

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

// Menu principal
bool menu();

// Opcion de crear una Nueva Pregunta
void nuevaPreg();
int pregTipo();
int pregTiempo();
int pregPuntos();
void guardarPreg(struct Pregunta *pregunta);

// Opcion de Modificar Pregunta
void modificarPreg();
void borrarPreg(int numPreg);

// Opcion de Conocer Estadisticas de una Pregunta
void estadisticasUso();

// Opcion de Porcentaje de Respuestas
void porcentajeResp();
float getPercentageVF(int vecesVerdad, int vecesFalso, int resCorrecta);
float getPercentageRM(int res1, int res2, int res3, int res4, int resCorrecta);

// Opcion de Seleccionar Pregunta
void selecPreg();

// Funciones adicionales
int cantPreg();
bool existePreg(int preg);
bool isEmpty();

// Funciones adicionales de archivos
struct Pregunta* getPreg(int numPreg);	// Busca cual es la linea de la pregunta
struct Pregunta* getFromLine(char linea [1000]);	// Guarda y retorna los datos de la pregunta
char * getFromRM(char linea[1000], bool desc);	// Dada una respuesta multiple, extrae la descripcion o las veces seleccionada
char * getFromVF(char linea[1000], bool consultar);	// Dada una respuesta Verdadero/Falso, retorna la cantidad de veces respondida


int main(int argc, char const *argv[])
{
	
	printf("\n=== Programa Administrador ===\n");
	
	printf("Inserte el nombre de un quiz existente o un nuevo nombre para crear un quiz: ");
	scanf("%[^\n]", archivo);
	printf("\n");

	strcat(archivo,".txt");
	
	file = fopen(archivo, "a");
	fclose(file);

	bool salir = false;
	while(salir != true){
		salir = menu();
	}
	
	return 0;
}

// ===============================================

bool menu(){

	int opcion;

	printf("\n======== %s =========\n", archivo);
	printf("Seleccione una de las siguientes opciones:\n\n");

	printf("1. Agregar nueva pregunta.\n");
	printf("2. Modificar pregunta existente.\n");
	printf("3. Conocer estadisticas de uso de una pregunta.\n");
	printf("4. Porcentaje de respuestas brindadas historicamente.\n");
	printf("5. Seleccionar preguntas que desea usar en el quiz.\n");
	printf("6. Salir.\n\n");

	scanf("%d", &opcion);
	printf("\n\n\n");
	
	switch(opcion){
		case 1:
			nuevaPreg();
			break;
		case 2:
			modificarPreg();
			break;
		case 3:
			estadisticasUso();
			break;
		case 4:
			porcentajeResp();
			break;
		case 5:
			selecPreg();
			break;
		case 6:
			return true;
	}

	return false;
}

// ===============================================

void nuevaPreg(){
	
	char descripcion [1000];
	int tipo;
	int resCorrecta;
	char vecesVF[50];
	char resMultiples[4][256];
	int tiempo;
	int puntos;
	
	strcpy(vecesVF, "NULL");
	strcpy(resMultiples[0], "NULL");
	strcpy(resMultiples[1], "NULL");
	strcpy(resMultiples[2], "NULL");
	strcpy(resMultiples[3], "NULL");

	printf("Pregunta #%d\n\n", cantPreg() + 1);
	
	// Descripcion
	printf("Escriba una descripcion para la pregunta: ");
	scanf(" %[^\n]", descripcion);
	printf("\n\n");
	
	// Tipo de pregunta
	tipo = pregTipo();
	
	// -------------------------------------------
	
	// Respuestas
	if (tipo == 1){	// Falso / Verdadero
		while (true){
			printf("Escriba 1 si la respuesta es verdadero o 2 si es falso: ");
			scanf("%d", &resCorrecta);
			printf("\n\n");
			
			if (resCorrecta == 1 || resCorrecta == 2)
				break;
			printf("Error, opcion incorrecta.\n\n");
		}
		
		strcpy(vecesVF, "0^0");	// Veces que se ha respondido Verdad o Falso
		
	} else {		// Seleccion multiple
		char desc [1000];
		for (int i = 0; i < 4; i++){
			printf("Escriba la descripcion para la respuesta #%d: ", i+1);
			scanf(" %[^\n]", desc);
			strcpy(resMultiples[i],desc);
			strcat(resMultiples[i], "^0|");
			bzero(desc, sizeof(desc));
			printf("\n\n");
		}
		while (true){
			printf("Escriba el numero (1,2,3,4) de la respuesta correcta: ");
			scanf("%d", &resCorrecta);
			printf("\n\n");
			
			resCorrecta--;
			
			if (resCorrecta == 0 || resCorrecta == 1 || resCorrecta == 2 || resCorrecta == 3)
				break;
			printf("Error, opcion incorrecta.\n\n");
		}
	}
	
	// -------------------------------------------
	
	// Tiempo
	tiempo = pregTiempo();
	
	//Puntos
	puntos = pregPuntos();
	
	// -------------------------------------------
	
	struct Pregunta* pregunta = malloc(sizeof(struct Pregunta));
	pregunta->numPreg = cantPreg() + 1;
	strcpy(pregunta->descripcion, descripcion);
	pregunta->tipo = tipo;
	pregunta->resCorrecta = resCorrecta;
	strcpy(pregunta->vecesVF, vecesVF);
	strcpy(pregunta->resMultiples[0], resMultiples[0]);
	strcpy(pregunta->resMultiples[1], resMultiples[1]);
	strcpy(pregunta->resMultiples[2], resMultiples[2]);
	strcpy(pregunta->resMultiples[3], resMultiples[3]);
	pregunta->tiempo = tiempo;
	pregunta->puntos = puntos;
	strcpy(pregunta->seVaAUsar, "false");
	pregunta->vecesUsada = 0;
	
	// Guardar en archivo
	guardarPreg(pregunta);
	
}

int pregTipo(){
	int tipo = 0;
	while (true){
		printf("Que tipo de pregunta desea hacer? Escriba 1 para una pregunta falso/verdadero o 2 para una pregunta de seleccion multiple: ");
		scanf("%d", &tipo);
		printf("\n\n");
		
		if (tipo == 1 || tipo == 2)
			break;
		printf("Error, opcion incorrecta.\n\n");
	}
	return tipo;
}

int pregTiempo(){
	int tiempo = 0;
	while (true){
		printf("Cuanto tiempo en segundos (5s a 60s) desea que dure la pregunta?: ");
		scanf("%d", &tiempo);
		printf("\n\n");
		
		if (tiempo >= 5 && tiempo <= 60)
			break;
		printf("Error, opcion incorrecta.\n\n");
	}
	return tiempo;
}

int pregPuntos(){
	int puntos = 0;
	while (true){
		printf("Cuantos puntos del quiz (1 a 30) desea que valga la pregunta?: ");
		scanf("%d", &puntos);
		printf("\n\n");
		
		if (puntos >= 1 && puntos <= 30)
			break;
		printf("Error, opcion incorrecta.\n\n");
	}
	return puntos;
}

void guardarPreg(struct Pregunta *pregunta){

	file = fopen(archivo, "a");
	char guardar [2000] = {};
	char num [10];
	
	fprintf(file, "%d%s%s%s%d%s%d%s%s%s", pregunta->numPreg, "$", pregunta->descripcion, "$", pregunta->tipo, "$", pregunta->resCorrecta, "$", pregunta->vecesVF, "$");
	
	if (pregunta->tipo == 2){	// Respuestas multiples
		for (int i = 0; i < 4; i++){
			// ^0| == 0 veces seleccionada
			fprintf(file, "%s", pregunta->resMultiples[i]);
		}
	} else {
		fprintf(file, "%s", "NULL");
	}
	
	fprintf(file, "%s%d%s%d%s%s%s%d%s", "$", pregunta->tiempo, "$", pregunta->puntos, "$", pregunta->seVaAUsar, "$", pregunta->vecesUsada, "$\n");	// No ha sido seleccionada y 0 veces seleccionada
	
	fclose(file);
}

// ===============================================


void modificarPreg(){
	if (isEmpty()){	// Valida que el archivo no esta vacio
		printf("El archivo esta vacio, no puede realizar esta operacion.\n\n");
		return;
	}

	int numPreg = 0;
	
	while (true){		// Valida que la pregunta existe
		printf("Seleccione el numero de pregunta que desea modificar: ");
		
		scanf("%d", &numPreg);
		printf("\n\n");
		
		if (existePreg(numPreg))
			break;
		printf("Error, la pregunta no existe.\n\n");
	}
	
	file = fopen(archivo, "r");
	
	
	struct Pregunta* pregunta = malloc(sizeof(struct Pregunta));
	pregunta = getPreg(numPreg);
	
	
	char res [1];
	char data[1000];
	int dataInt = 0;

	printf("Pregunta #%d\n\n", pregunta->numPreg);
	
	// Descripcion -------------------------------------------
	
	printf("La descripcion actual es: %s\n\n", pregunta->descripcion);
	printf("Desea cambiar la descripcion? Si es asi, escriba la letra Y: ");
	scanf(" %c", res);
	printf("\n\n");
	
	if (strcmp(res, "Y") == 0){
		printf("Escriba una descripcion para la pregunta: ");
		scanf(" %[^\n]", data);
		strcpy(pregunta->descripcion, data);
		bzero(data, sizeof(data));
		printf("\n\n");
	}
	
	
	// Tipo de pregunta -------------------------------------------
	
	bool cambioTipo = false;

	printf("El tipo de pregunta actual es: ");
	
	if (pregunta->tipo == 1)
		printf("Falso/Verdadero.\n\n");
	else
		printf("Respuesta Multiple.\n\n");
		
	printf("Desea cambiar el tipo de pregunta? Si es asi, escriba la letra Y: ");
	scanf(" %c", res);
	printf("\n\n");
	
	if (strcmp(res, "Y") == 0){
		cambioTipo = true;
		if (pregunta->tipo == 2)
			pregunta->tipo = 1;
		else
			pregunta->tipo = 2;
	}
	
	// Respuestas -------------------------------------------
	
	if (pregunta->tipo == 1 && cambioTipo){	// Nuevo Falso / Verdadero
		while (true){
			printf("Escriba 1 si la respuesta es verdadero o 2 si es falso: ");
			scanf("%d", &dataInt);
			printf("\n\n");
			
			strcpy(pregunta->vecesVF, "0^0");
			if (dataInt == 1 || dataInt == 2)
				break;
			printf("Error, opcion incorrecta.\n\n");
		}
		
		pregunta->resCorrecta = dataInt;
		dataInt = 0;
		
	} else if (pregunta->tipo == 1 && !cambioTipo){		// Actualiza Falso / Verdadero
		printf("La respuesta actual es: ");
		
		if (pregunta->resCorrecta == 1)
			printf("Verdadero.\n\n");
		else
			printf("Falso.\n\n");
		
		printf("Desea cambiar la respuesta? Si es asi, escriba la letra Y: ");
		scanf(" %c", res);
		printf("\n\n");
		
		if (strcmp(res, "Y") == 0){
			strcpy(pregunta->vecesVF, "0^0");
			if (pregunta->resCorrecta == 2)
				pregunta->resCorrecta = 1;
			else
				pregunta->resCorrecta = 2;
		}
		
	} else if (pregunta->tipo == 2 && cambioTipo){		// Nuevo Respuesta multiple
		strcpy(pregunta->vecesVF, "NULL");
		for (int i = 0; i < 4; i++){
			printf("Escriba la descripcion para la respuesta #%d: ", i+1);
			scanf(" %[^\n]", data);
			
			strcpy(pregunta->resMultiples[i],data);
			strcat(pregunta->resMultiples[i], "^0|");
			bzero(data, sizeof(data));
			
			printf("\n\n");
		}
		while (true){
			printf("Escriba el numero (1,2,3,4) de la respuesta correcta: ");
			scanf("%d", &dataInt);
			printf("\n\n");
			
			dataInt--;
			if (dataInt == 1 || dataInt == 2 || dataInt == 3 || dataInt == 4)
				break;
			printf("Error, opcion incorrecta.\n\n");
		}
		
		pregunta->resCorrecta = dataInt;
		dataInt = 0;
		
	} else {	// Actualiza Respuesta multiple

		for (int i = 0; i < 4; i++){
			
			bzero(data, sizeof(data));
			bzero(res, sizeof(res));
			
			strcpy(data, getFromRM(pregunta->resMultiples[i], true));

			printf("La descripcion actual para la respuesta #%d es: %s\n\n", i+1, data);
			printf("Desea cambiar la descripcion? Si es asi, escriba la letra Y: ");
			scanf(" %s", res);
			printf("\n\n");

			if (strcmp(res, "Y") == 0){
				bzero(pregunta->resMultiples[i], sizeof(pregunta->resMultiples[i]));
				bzero(data, sizeof(data));
				
				printf("Escriba la descripcion para la respuesta #%d: ", i+1);
				scanf(" %[^\n]", data);
				strcpy(pregunta->resMultiples[i],data);
				strcat(pregunta->resMultiples[i],"^0|");
				bzero(data, sizeof(data));
				printf("\n\n");
			}
			
		}
		while (true){
			printf("Escriba el numero (1,2,3,4) de la respuesta correcta: ");
			scanf("%d", &dataInt);
			printf("\n\n");
			
			dataInt--;
			
			if (dataInt == 0 || dataInt == 1 || dataInt == 2 || dataInt == 3)
				break;
			printf("Error, opcion incorrecta.\n\n");
		}
		
		pregunta->resCorrecta = dataInt;
		dataInt = 0;
		
	}
	
	// Tiempo y Puntos -------------------------------------------
	printf("El tiempo actual es: %d\n\n", pregunta->tiempo);
	printf("Desea cambiar el tiempo de la pregunta? Si es asi, escriba la letra Y: ");
	scanf(" %c", res);
	printf("\n\n");
	
	if (strcmp(res, "Y") == 0){
		while (true){
			printf("Cuanto tiempo en segundos (5s a 60s) desea que dure la pregunta?: ");
			scanf("%d", &dataInt);
			printf("\n\n");
			
			if (dataInt >= 5 && dataInt <= 60)
				break;
			printf("Error, opcion incorrecta.\n\n");
		}
		
		pregunta->tiempo = dataInt;
		dataInt = 0;
		
	}
	
	printf("Los puntos actuales de la pregunta son: %d\n\n", pregunta->puntos);
	printf("Desea cambiar los puntos de la pregunta? Si es asi, escriba la letra Y: ");
	scanf(" %c", res);
	printf("\n");
	
	if (strcmp(res, "Y") == 0){
		while (true){
			printf("Cuantos puntos del quiz (1 a 30) desea que valga la pregunta?: ");
			scanf("%d", &dataInt);
			printf("\n\n");
			
			if (dataInt >= 1 && dataInt <= 30)
				break;
			printf("Error, opcion incorrecta.\n\n");
		}
		
		pregunta->puntos = dataInt;
		dataInt = 0;
		
	}
	
	fclose(file);
	
	borrarPreg(numPreg);
	guardarPreg(pregunta);
}

void borrarPreg(int numPreg){
	char data [10000];
	char linea [1000];
	char num [1000];
	int i;
	bool skip = false;
	
	bzero(data, sizeof(data));
	bzero(linea, sizeof(linea));
	bzero(num, sizeof(num));
	
	file = fopen(archivo, "r");
	
	while (fscanf(file, " %[^\n]", linea) != EOF){	// Para cada linea
		num[0] = '\0';
		for (i = 0; i < 1000; i++){		// Para cada caracter
			if (linea[i] == '$'){
				
				if (atoi(num) == numPreg){
					skip = true;
					break;
				}
				break;			// Ya leyo el numero, siguiente linea
			}
			
			strncat(num, &linea[i], 1);
		}
		
		if (skip){
			skip = false;
			continue;
		}
		
		strcat(linea, "\n");
		strcat(data, linea);
		bzero(linea, sizeof(linea));
	}
	
	fclose(file);
	
	file = fopen(archivo, "w");
	
	fprintf(file, "%s", data);
	fclose(file);
}

// ===============================================

void estadisticasUso(){

	if (isEmpty()){	// Valida que el archivo no esta vacio
		printf("El archivo esta vacio, no puede realizar esta operacion.\n\n");
		return;
	}

	int numPreg;
	
	while (true){		// Valida que la pregunta existe
		printf("Seleccione el numero de pregunta que desea consultar: ");
		scanf("%d", &numPreg);
		printf("\n\n");
		
		if (existePreg(numPreg))
			break;
		printf("Error, la pregunta no existe.\n\n");
	}
	
	struct Pregunta* pregunta = malloc(sizeof(struct Pregunta));
	pregunta = getPreg(numPreg);

	printf("La pregunta se ha usado %d veces.\n\n", pregunta->vecesUsada);
}

// ===============================================

void porcentajeResp(){

	if (isEmpty()){	// Valida que el archivo no esta vacio
		printf("El archivo esta vacio, no puede realizar esta operacion.\n\n");
		return;
	}

	int numPreg;
	
	while (true){		// Valida que la pregunta existe
		printf("Seleccione el numero de pregunta que desea consultar: ");
		scanf("%d", &numPreg);
		printf("\n\n");
		
		if (existePreg(numPreg))
			break;
		printf("Error, la pregunta no existe.\n\n");
	}

	
	struct Pregunta* pregunta = malloc(sizeof(struct Pregunta));
	pregunta = getPreg(numPreg);
	float porcentaje = 0;
	
	printf("La pregunta #%d con descripcion: %s , se ha respondido ", pregunta->numPreg, pregunta->descripcion);
	
	if (pregunta->tipo == 1){
		porcentaje = getPercentageVF(atoi(getFromVF(pregunta->vecesVF, true)), atoi(getFromVF(pregunta->vecesVF, false)), pregunta->resCorrecta);
		
		printf("%d veces y tiene un porcentaje de respuestas correctas del %f porciento, con la respuesta correcta como ", atoi(getFromVF(pregunta->vecesVF, true))+atoi(getFromVF(pregunta->vecesVF, false)), porcentaje);
		
		if (pregunta->resCorrecta == 1)
			printf("'Verdadero'.\n\n");
		else
			printf("'Falso'.\n\n");
	} else {
		
		porcentaje = getPercentageRM(atoi(getFromRM(pregunta->resMultiples[0], false)), atoi(getFromRM(pregunta->resMultiples[1], false)), atoi(getFromRM(pregunta->resMultiples[2], false)), atoi(getFromRM(pregunta->resMultiples[3], false)), pregunta->resCorrecta);
		
		printf("%d veces y tiene un porcentaje de respuestas correctas del %f porciento, con la respuesta correcta con la siguiente descripcion: ", atoi(getFromRM(pregunta->resMultiples[0], false))+atoi(getFromRM(pregunta->resMultiples[1], false))+atoi(getFromRM(pregunta->resMultiples[2], false))+atoi(getFromRM(pregunta->resMultiples[3], false)), porcentaje);
		
		int i;
		switch (pregunta->resCorrecta){
			case 0:
				i = 0;
				break;
			case 1:
				i = 1;
				break;
			case 2:
				i = 2;
				break;
			case 3:
				i = 3;
		}
		
		printf("'%s'.\n\n", getFromRM(pregunta->resMultiples[i], true));
		
	}
}

float getPercentageVF(int vecesVerdad, int vecesFalso, int resCorrecta){
	int total = vecesVerdad + vecesFalso;
	
	if (resCorrecta == 1){
		if (vecesVerdad == 0 && total == 0)
			return 0;
		return ((float)vecesVerdad/(float)total)*100;
	} else {
		if (vecesFalso == 0 && total == 0)
			return 0;
		return ((float)vecesFalso/(float)total)*100;
	}
}

float getPercentageRM(int res1, int res2, int res3, int res4, int resCorrecta){
	int total = res1 + res2 + res3 + res4;
	
	switch (resCorrecta){
		case 0:
			if (res1 == 0 && total == 0)
				return 0;
			return ((float)res1/(float)total)*100;
		case 1:
			if (res2 == 0 && total == 0)
				return 0;
			return ((float)res2/(float)total)*100;
		case 2:
			if (res3 == 0 && total == 0)
				return 0;
			return ((float)res3/(float)total)*100;
		case 3:
			if (res4 == 0 && total == 0)
				return 0;
			return ((float)res4/(float)total)*100;
	}
	
	return 0;
}

// ===============================================

void selecPreg(){
	
	struct Pregunta* pregunta = malloc(sizeof(struct Pregunta));
	int numPreg;
	bool salir = false;
	
	if (isEmpty()){	// Valida que el archivo no esta vacio
		printf("El archivo esta vacio, no puede realizar esta operacion.\n\n");
		return;
	}
	
	while (!salir){
		printf("Seleccione el numero de pregunta que marcar o desmarcar: ");
		scanf("%d", &numPreg);
		printf("\n\n");
		
		if (existePreg(numPreg)){
			pregunta = getPreg(numPreg);
			if (strcmp(pregunta->seVaAUsar, "false") == 0){
				strcpy(pregunta->seVaAUsar, "true");
				printf("La pregunta #%d se ha marcado y si se usara.\n", numPreg);
			} else {
				strcpy(pregunta->seVaAUsar, "false");
				printf("La pregunta #%d se ha desmarcado y no se usara.\n", numPreg);
			}
			
			borrarPreg(numPreg);
			guardarPreg(pregunta);
			
		} else {
			printf("Error, la pregunta no existe.\n\n");
		}
		
		printf("Desea salir? Presione 1: ");
		scanf("%d", &numPreg);
		printf("\n\n");
		
		if (numPreg == 1)
			salir = true;
	}
}

// ===============================================

int cantPreg(){

	if (isEmpty())
		return 0;

	size_t len = 0;
	ssize_t read;

	int contador = 0;
	char* line = NULL;
	
	file = fopen(archivo, "r");
	
	while ((read = getline(&line, &len, file)) != -1)
		contador++;
		
	fclose(file);

	return contador;
}

bool existePreg(int preg){

	char linea [1000];
	char num [1000];
	int i;
	
	file = fopen(archivo, "r");
	
	while (fscanf(file, " %[^\n]", linea) != EOF){	// Para cada linea
		num[0] = '\0';
		for (i = 0; i < 1000; i++){		// Para cada caracter
			if (linea[i] == '$'){
				if (atoi(num) == preg){
					fclose(file);
					return true;
				}
				break;			// Ya leyo el numero, siguiente linea
			}
			
			strncat(num, &linea[i], 1);
		}
	}
	
	fclose(file);
	
	return false;
}

bool isEmpty(){
	file = fopen(archivo, "r");
	fseek (file, 0, SEEK_END);
	if (0 == ftell(file))
		return true;
	fclose(file);
	return false;
}

// ===============================================

struct Pregunta* getPreg(int numPreg){
	
	char linea [1000];
	char current [1000];
	int i;
	
	file = fopen(archivo, "r");
	
	bzero(linea, sizeof(linea));
	
	while (fscanf(file, " %[^\n]", linea) != EOF){	// Para cada linea
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
	
	fclose(file);
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

char * getFromVF(char linea[1000], bool consultar){
	strcat(linea, "$");
	char * current = malloc(sizeof *current);
	bzero(current, sizeof(current));
	
	for (int i = 0; i < 1000; i++){
		if (linea[i] == '^'){
			if (consultar){
				return current;
			} else {
				bzero(current, sizeof(current));
				continue;
			}
		}
		
		if (linea[i] == '$')
			return current;
		
		strncat(current, &linea[i], 1);
	
	}
	
	return NULL;
}








