#include "Trama.h"

//string -> trama

//trama -> string 
void setTramaString (Trama trama, int fd) {
    char * string = (char *)malloc(sizeof(char) * 256); // tramas fijas de 256 Bytes

    string[0] = trama.type;//0x02

    //asprintf(&string, "%s%s", string, header);
    string = concatStringsWithoutNull(string, header);
  
    char* length = malloc(2 * sizeof(char));
    intToBytes(/*trama.length*/2, length);
  
    printf("BYTES: -%02X- -%02X-\n", length[0], length[1]);
    printf("BYTES: -%c- -%c-\n", length[0], length[1]);
  
    // teniendo en cuenta que el strlen devuelve un +1 a la posicion real
    int offset = strlen(trama.header) + 2;
    printf("OFFSET: %d\n", offset);
  
    //string[++offset] = 0b00010000;//length[0];
    string[++offset] = length[0];
    //string[++offset] = 0b11110000;//length[1];
    string[++offset] = length[1];

    // no tocar
    for (int i = 0; i < trama.length; i++) {
        string[++offset] = trama.data[i];
    }
    printf("STRING END: -%s-\n", string);
  
    write(fd, string, 5 + strlen(header) + trama.length); 

  
  	free(string);
  	free(length);
  	free(header);
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