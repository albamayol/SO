#include "Trama.h"

//string -> trama

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

  /*
  char letra = 'K';
  for(int i = 0; i < 256; i++) {
    printf("%d%c\n", i, string[i]);
    if (string[i] == '\0') {
      write(1, &letra, sizeof(char));
    } else {
      write(1, &string[i], sizeof(char));
    }
    

  }*/



  for(int i = 0; i < 256; i++) {
    write(1, &string[i], sizeof(char));
  }

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

  trama.header = malloc(sizeof(char) * (Trama.header_length));
  memset(trama.header, 0, (Trama.header_length));
  //TODO Copiar byte a byte en bucle
  for(int i = 0; i < trama.header_length; i++) {
    trama.header[i] = header[i];
    j++;
  }
  
  
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