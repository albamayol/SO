/*
Autores:
    Alba Mayol Lozano -->alba.mayol
    Kevin Eljarrat Ohayon --> kevin.eljarrat
    LaSalle - Sistemes Operatius
*/

#ifndef _TRAMA_H_
#define _TRAMA_H_

#include "Global.h"

typedef struct {
    char type;
    short header_length; 
    char *header;
    char *data;
} Trama;

typedef struct {
    int initialized;
    Trama trama;
} TramaExtended;

#define printF(x) write(1, x, strlen(x))

char* createString3Params(char* param1, char* param2, char* param3);
char* createString4Params(char* param1, char* param2, char* param3, char *param4);
void shortToChars(short valor, char *cadena);
void setTramaString (Trama trama, int fd);
TramaExtended readTrama(int fd);
Trama TramaCreate (char type, char *header, char *data, size_t size);
void freeTrama(Trama *trama);

#endif