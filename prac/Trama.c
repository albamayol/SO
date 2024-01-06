#include "Trama.h"

void freeTrama(Trama *trama) {
    if (trama->data != NULL) {
        free(trama->data);
        trama->data = NULL;  
    }
    if (trama->header != NULL) {
        free(trama->header);
        trama->header = NULL;
        
    }
}

void shortToChars(short valor, char *cadena) {
  cadena[1] = (char)(valor & 0xFF);        // Obtener el byte de menor peso
  cadena[0] = (char)((valor >> 8) & 0xFF); // Obtener el byte de mayor peso
}

char* createString3Params(char* param1, char* param2, char* param3) { 
  char *aux = NULL;
  size_t length = strlen(param1) + strlen(param2) + strlen(param3) + 2 + 1; 
  aux = (char *) malloc(sizeof(char) * length);
  memset(aux, 0, length);
  strcpy(aux, param1);
  strcat(aux, "&"); 
  strcat(aux, param2);
  strcat(aux, "&");
  strcat(aux, param3); 
  return aux;
}

char* createString4Params(char* param1, char* param2, char* param3, char *param4) { 
  char *aux = NULL;
  size_t length = strlen(param1) + strlen(param2) + strlen(param3) + strlen(param4) + 3 + 1; 
  aux = (char *) malloc(sizeof(char) * length);
  memset(aux, 0, length);
  strcpy(aux, param1);
  strcat(aux, "&"); 
  strcat(aux, param2);
  strcat(aux, "&");
  strcat(aux, param3); 
  strcat(aux, "&");
  strcat(aux, param4); 
  return aux;
}

void setTramaString(Trama trama, int fd) {
  char string[256] = {0}; 

  string[0] = trama.type;
  
  char header_len[2] = {0};
  shortToChars(trama.header_length, header_len);

  string[1] = header_len[1];
  string[2] = header_len[0];

  //posiciones 0, 1 y 2 ya ocupadas
  int offset = 3;

  //HEADER
  for (int i = 0; i < trama.header_length; i++) {
    string[offset] = trama.header[i];
    ++offset;
  }

  //DATA
  int data_len = 256 - 3 - trama.header_length; 
  
  for(int i = 0; i < data_len; i++) {
    string[offset] = trama.data[i];
    ++offset;
  }
  
  write(fd, string, 256);

  freeTrama(&trama);
}

Trama TramaCreate (char type, char *header, char *data, size_t size) {
  Trama trama;

  trama.type = type;
  trama.header_length = strlen(header);
  
  trama.header = malloc(sizeof(char) * (trama.header_length + 1));
  memset(trama.header, 0, (trama.header_length + 1));
  strcpy(trama.header, header);
  
  int sizeData = 256 - 3 - trama.header_length; 
  trama.data = malloc(sizeof(char) * (sizeData));
  memset(trama.data, '~', sizeData); //Padding

  for (size_t i = 0; i < size; i++) {
      trama.data[i] = data[i];
  }

  return trama;
}

void inicializarTrama(TramaExtended *tramaExtended) {
  tramaExtended->trama.type = ' ';
  tramaExtended->trama.header_length = 0;
  tramaExtended->trama.header = NULL;
  tramaExtended->trama.data = NULL;
  tramaExtended->initialized = 0;
}

TramaExtended readTrama(int fd) {
  //Trama trama;
  TramaExtended tramaExtended;
  char *buffer = NULL;
  inicializarTrama(&tramaExtended);
  size_t checkPoole = 0;

  checkPoole = read(fd, &tramaExtended.trama.type, sizeof(char));  
  if (checkPoole <= 0) {
    tramaExtended.initialized = 1;
  }

  read(fd, &tramaExtended.trama.header_length, sizeof(unsigned short));
  tramaExtended.trama.header = malloc((tramaExtended.trama.header_length+1) * sizeof(char)); 

  if (tramaExtended.trama.header != NULL) {
    read(fd, tramaExtended.trama.header, tramaExtended.trama.header_length);
    tramaExtended.trama.header[tramaExtended.trama.header_length] = '\0'; 
  } 

  size_t sizeData = 256 - 3 - tramaExtended.trama.header_length;

  buffer = (char *) malloc(sizeof(char) * sizeData);
  tramaExtended.trama.data = (char *) malloc(sizeof(char) * sizeData + 1);
  memset(tramaExtended.trama.data, '~', sizeData);

  read(fd, buffer, sizeof(char) * sizeData);
  size_t i = 0;
  for (i = 0; i < sizeData; i++) {
    tramaExtended.trama.data[i] = buffer[i];
  }
  tramaExtended.trama.data[i] = '\0';
  freeString(&buffer);

  return tramaExtended;
}