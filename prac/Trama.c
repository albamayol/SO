#include "Trama.h"

void shortToChars(short valor, char *cadena) { 
  cadena[1] = (char)(valor & 0xFF);        // Obtener el byte de menor peso
  cadena[0] = (char)((valor >> 8) & 0xFF); // Obtener el byte de mayor peso
}

short charsToShort(char char1, char char2) {
    return ((short)(char1) << 8) | (short)(char2);
}

char* anadirClaudators(char *charheader) {
    char *newHeader = NULL;
    asprintf(&newHeader, "[%s]", charheader);

    return newHeader;
}

char* createString3Params(char* param1, char* param2, char* param3) {
    //dPoole.serverName&dPoole.ipServer&dPoole.puertoServer
    char *aux = NULL;

    int length = strlen(param1) + strlen(param2) + strlen(param3) + 3 + 1;
    aux = (char *) malloc(sizeof(char) * length);

    for (int i = 0; i < length; i++) {
        aux[i] = '\0';
    }

    strcpy(aux, param1);
    strcat(aux, "&"); 
    strcat(aux, param2);
    strcat(aux, "&");
    strcat(aux, param3); 

    return aux;
}

//string -> trama
Trama setStringTrama(char *string) {
  Trama trama;
  int i;

  // Gestion del Type
  trama.type = string[0];

  // Gestion del Header Length
  trama.header_length = charsToShort(string[1], string[2]);
  

  // Gestion del Header
  trama.header = (char *)malloc((trama.header_length + 1) * sizeof(char)); 

  for (i = 0; i < trama.header_length; i++) {
    trama.header[i] = string[i + 3];
  }

  trama.header[i] = '\0';

  // Gestion del Data
  int dataSize = 256 - 3 - trama.header_length;

  trama.data = (char *)malloc((dataSize + 1) * sizeof(char)); 

  for (i = 0; i < dataSize; i++) {
    trama.data[i] = string[i + 3 + trama.header_length];
  }

  trama.data[i] = '\0';

  return trama;
}




//trama -> string 
void setTramaString(Trama trama, int fd) {
  char *string = malloc((trama.header_length + strlen(trama.data) + 3) * sizeof(char)); //3 --> 1Byte type, 2Bytes header_length

  string[0] = trama.type;

  char *header_len = malloc(2 * sizeof(char));
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

  //TESTING
  write(1, "resultat construcció string trama: \n", strlen("resultat construcció string trama: \n"));
  write(1, string, strlen(string));
  
  write(fd, string, strlen(string));

  free(string);
  string = NULL;
  free(header_len);
  header_len = NULL;
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
  memset(trama.data, 0, sizeData); //Padding
  strcpy(trama.data, data); //no redimensiona

  return trama;
}

void freeTrama(Trama trama) {
  free(trama.data);
  trama.data = NULL;
  free(trama.header);
  trama.header = NULL;
}