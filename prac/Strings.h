/*
Autors:
    Alba Mayol Lozano -->alba.mayol
    Kevin Eljarrat Ohayon --> kevin.eljarrat
    LaSalle - Sistemes Operatius
Data creació: 23/05/24
Data última modificació: 23/05/24
*/

#ifndef _STRINGS_H_
#define _STRINGS_H_

#define _GNU_SOURCE

#include <string.h>
#include <strings.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>

char* read_until(int fd, char delimiter);
char* read_until_string(char *string, char delimiter);
void removeExtraSpaces(char *comanda);
void cleanPadding(char* string, char delimiter);
char * to_upper(char * str);
char * verifyClientName(char * clienteNameAux);
void freeString(char **string);
char* convertIntToString(int num);
char* readNumChars(char *string, int inicio, int final);
char* readUntilFromIndex(char *string, int *inicio, char delimiter, char *final, char delimitadorFinal);


#endif