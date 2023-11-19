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
    char *msg = NULL;
    asprintf(&msg, "param1: \"%s\"\n", param1);
    printF(msg);
    free(msg);
    msg = NULL;
    asprintf(&msg, "param2: \"%s\"\n", param2);
    printF(msg);
    free(msg);
    msg = NULL;
    asprintf(&msg, "param3: \"%s\"\n", param3);
    printF(msg);
    free(msg);
    msg = NULL;
  


    //dPoole.serverName &dPoole.ipServer &dPoole.puertoServer 
    char *aux = NULL;

    //hola\0 --> 4
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
  //write(1, "TYPE\n", strlen("TYPE\n"));
  //printf("type hex: %02x\n", trama.type);
  
  string[0] = trama.type;
  //printf("type from string: %02x\n", string[0]);
  

  //write(1, "HEADER_LEN\n", strlen("HEADER_LEN\n"));
  //printf("%hd\n", trama.header_length);
  char *header_len = malloc(2 * sizeof(char));
  shortToChars(trama.header_length, header_len);

  string[1] = header_len[1];
  string[2] = header_len[0];

  //printf("%d\n", string[1]);
  //printf("%d\n", string[2]);
 
  //posiciones 0, 1 y 2 ya ocupadas
  int offset = 3;

  //write(1, "HEADER\n", strlen("HEADER\n"));
  //write(1, trama.header, strlen(trama.header));
  //write(1, "\n", strlen("\n"));
  //HEADER
  for (int i = 0; i < trama.header_length; i++) {
    string[offset] = trama.header[i];
    //printf("%c\n", string[offset]);
    ++offset;
  }

  //DATA
  //write(1, "DATA\n", strlen("DATA\n"));
  
  int data_len = 256 - 3 - trama.header_length; 

  //write(1, trama.data, data_len);
  //write(1, "\n", strlen("\n"));
  

  for(int i = 0; i < data_len; i++) {
    string[offset] = trama.data[i];
    //printf("%d\n", string[offset]);
    //printf("%c\n", string[offset]);
    ++offset;
  }
  

  //TESTING
  write(1, "resultat construcció string trama: \n", strlen("resultat construcció string trama: \n"));
  for(int i = 0; i < 256; i++) {
    printf("%c\n", string[i]);
  }
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
  int sizeDataString = 0;
  sizeDataString = strlen(data);
  for(int i = 0; i < sizeDataString; i++) {
      trama.data[i] = data[i];
  }
  //strcpy(trama.data, data); //no redimensiona

  return trama;
}

void freeTrama(Trama trama) {
  free(trama.data);
  trama.data = NULL;
  free(trama.header);
  trama.header = NULL;
}