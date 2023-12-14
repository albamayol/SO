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
  int length = strlen(param1) + strlen(param2) + strlen(param3) + 3 + 1;
  aux = (char *) malloc(sizeof(char) * length);
  memset(aux, 0, length);
  strcpy(aux, param1);
  strcat(aux, "&"); 
  strcat(aux, param2);
  strcat(aux, "&");
  strcat(aux, param3); 
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

Trama TramaCreate (char type, char *header, char *data) {
  Trama trama;

  trama.type = type;
  trama.header_length = strlen(header);
  
  trama.header = malloc(sizeof(char) * (trama.header_length + 1));
  memset(trama.header, 0, (trama.header_length + 1));
  strcpy(trama.header, header);
  
  int sizeData = 256 - 3 - trama.header_length;
  trama.data = malloc(sizeof(char) * (sizeData));
  memset(trama.data, '~', sizeData); //Padding
  int sizeDataString = 0;
  sizeDataString = strlen(data);

  for (int i = 0; i < sizeDataString; i++) {
      trama.data[i] = data[i];
  }

  return trama;
}

void inicializarTrama(Trama *trama) {
  trama->type = ' ';
  trama->header_length = 0;
  trama->header = NULL;
  trama->data = NULL;
}

Trama readTrama(int fd) {
  Trama trama;
  char *buffer = NULL;
  inicializarTrama(&trama);

  read(fd, &trama.type, sizeof(char));    
  read(fd, &trama.header_length, sizeof(unsigned short));
  trama.header = malloc((trama.header_length+1) * sizeof(char)); 

  if (trama.header != NULL) {
    read(fd, trama.header, trama.header_length);
    trama.header[trama.header_length] = '\0'; 
  } 

  
  trama.data = read_until(fd, '~');
  int sizeData = 256 - 3 - trama.header_length - strlen(trama.data) - 1; //cantidad restantes de '~'

  buffer = (char *) malloc(sizeof(char) * (sizeData));
  read(fd, buffer, sizeData * sizeof(char));

  freeString(&buffer);

  return trama;
}