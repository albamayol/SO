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
  int i = 0;
  
  // Gestion del Type
  trama.type = string[0];

  // Gestion del Header Length
  trama.header_length = charsToShort(string[2], string[1]);

  // Gestion del Header
  trama.header = (char *)malloc((trama.header_length + 1) * sizeof(char)); 

  for (i = 0; i < trama.header_length-1; i++) {
    trama.header[i] = string[i + 3];
  }
  trama.header[i + 3] = '\0';


  //DATA
  int dataSize = 256 - 3 - trama.header_length;
  trama.data = (char *)malloc((dataSize + 1) * sizeof(char)); 
  for (i = 0; i < dataSize; i++) {

    trama.data[i] = string[i + 2 + trama.header_length];
  }
  trama.data[i] = '\0';

  return trama;
}

//trama -> string 
void setTramaString(Trama trama, int fd) {
  char *string = malloc((trama.header_length + strlen(trama.data) + 3) * sizeof(char)); //3 --> 1Byte type, 2Bytes header_length
  
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
    //write(1, &string[offset], sizeof(char)); //LO HACE BIEN
    ++offset;
  }

  //DATA
  
  int data_len = 256 - 3 - trama.header_length; 
  

  for(int i = 0; i < data_len; i++) {
    string[offset] = trama.data[i];
    ++offset;
  }
  
  //write(1, string, 256); //SE ENVIA BIEN
  write(fd, string, 256);

  free(string);
  string = NULL;
  
}

Trama TramaCreate (char type, char *header, char *data) {
  Trama trama;


  trama.type = type;

  trama.header_length = strlen(header);
  
  trama.header = malloc(sizeof(char) * (trama.header_length + 1));
  memset(trama.header, 0, (trama.header_length + 1));
  strcpy(trama.header, header);

  //write(1, trama.header, strlen(trama.header)); //ESTO LO HACE BIEN
  
  int sizeData = 256 - 3 - trama.header_length;
  //write(1, data, sizeData *sizeof(char));
  trama.data = malloc(sizeof(char) * (sizeData));
  memset(trama.data, '~', sizeData); //Padding
  //strcpy(trama.data, data); //no redimensiona
  int sizeDataString = 0;
  sizeDataString = strlen(data);

  //char c = 'K';
  for(int i = 0; i < sizeDataString; i++) {
      trama.data[i] = data[i];
      /*if (trama.data[i] == '\0') {
        write(1, &c, sizeof(char));
      }*/
  }
  //write(1, trama.data, strlen(trama.data));
  return trama;
}

Trama readTrama(int fd) {
  Trama trama;
  read(fd, &trama.type, sizeof(char));    
  read(fd, &trama.header_length, sizeof(short));
  trama.header = malloc(sizeof(char)*(trama.header_length+1));
  read(fd, trama.header, trama.header_length);
  trama.header[trama.header_length] = '\0';
  trama.data = read_until(fd, '~');

  return trama;
}

void freeTrama(Trama *trama) {
  free(trama->data);
  free(trama->header);
  trama->data = NULL;
  trama->header = NULL;
}