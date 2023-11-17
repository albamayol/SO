#include "Trama.h"

//string -> trama
Trama setStringTrama(char *string) {
  //1 01 kevi lijasdnfajsdnfjksandfkjasndfkjansdkjfnasdfdnfjnasdfjnasdjfnvjdsfnvjksdnvjk
  //ej:       00000001              0000000000000100             0000000011111110000000011111111        256-7 = 258Bytes
  //          type = 1Byte      header_length = 2Bytes                Header = 4Bytes                    DATA
  Trama trama;
  int i;

  // Gestion del Type
  trama.type = string[0];

  // Gestion del Header Length
  char string[2] = strcat(string[1], string[2]);

  trama.header_length = charsToShort(string);

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

void shortToChars(short valor, char *cadena) {
  char cadena[1] = (char)(valor & 0xFF);        // Obtener el byte de menor peso
  char cadena[0] = (char)((valor >> 8) & 0xFF); // Obtener el byte de mayor peso
}

short charsToShort(char *cadena) {
    return (short)atoi(cadena);
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
  for(int i = 0; i < strlen(trama.data); i++) {
    string[offset] = trama.data[i];
    ++offset;
  }

  //TESTING
  write(1, "resultat construcció string trama: \n", strlen("resultat construcció string trama: \n"));
  write(1, string, strlen(string));
  
  write(fd, string, strlen(string));

  free(string);
  free(header_len);
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
  free(trama.header);
  trama.data = NULL;
  trama.header = NULL;
}