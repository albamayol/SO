#include "Trama.h"

//string -> trama

//trama -> string 
void setTramaString(Trama trama, int fd) {
  char *string = malloc((trama.header_length + strlen(trama.data) + 3) * sizeof(char)); //3 --> 1Byte type, 2Bytes header_length

  string[0] = trama.type;
 
  char *header_len = malloc(2 * sizeof(char));
  intToBytes(trama.header, header_len);

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

  trama.header = malloc(sizeof(char) * (Trama.header_length+1));
  memset(trama.header, 0, (Trama.header_length+1));
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