#ifndef _TRAMA_H_
#define _TRAMA_H_

#include "Global.h"

typedef struct {
    char type;
    short header_length; 
    char *header;
    char *data;
} Trama;

#define printF(x) write(1, x, strlen(x))

char* anadirClaudators(char *charheader);
char* createString3Params(char* param1, char* param2, char* param3);
Trama setStringTrama(char *string);
void shortToChars(short valor, char *cadena);
void setTramaString (Trama trama, int fd);
Trama readTrama(int fd);
Trama TramaCreate (char type, char *header, char *data);
void freeTrama(Trama *trama);

#endif