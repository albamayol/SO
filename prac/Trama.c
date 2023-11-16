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
  int valor = (string[1] - '0') * 10 + (string[2] - '0');
  //char short

  

  

  trama.header_length = valor;



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
  //0000000000000001
  01
  char cadena[1] = (char)(valor & 0xFF);        // Obtener el byte de menor peso
  char cadena[0] = (char)((valor >> 8) & 0xFF); // Obtener el byte de mayor peso
}

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