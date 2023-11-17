#ifndef _TRAMA_H_
#define _TRAMA_H_

//HEADER's defines
#define NEW_POOLE "NEW_POOLE"
#define CON_OK "CON_OK"
#define CON_KO "CON_KO"
#define NEW_BOWMAN "NEW_BOWMAN"
#define LIST_SONGS "LIST_SONGS"
#define SONGS_RESPONSE "SONGS_RESPONSE"
#define LIST_PLAYLISTS "LIST_PLAYLISTS"
#define PLAYLISTS_RESPONSE "PLAYLISTS_RESPONSE"
#define DOWNLOAD_SONG "DOWNLOAD_SONG"
#define DOWNLOAD_LIST "DOWNLOAD_LIST"
#define NEW_FILE "NEW_FILE"
#define FILE_DATA "FILE_DATA"
#define CHECK_OK "CHECK_OK"
#define CHECK_KO "CHECK_KO"
#define EXIT "EXIT"
#define CONOK "CONOK"
#define CONKO "CONKO"
#define UNKNOWN "UNKNOWN"

#define _GNU_SOURCE

#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <strings.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdint.h>

typedef struct {
    char type;
    short header_length; 
    char *header;
    char *data;
} Trama;

char* anadirClaudators(char *charheader);
char* createString3Params(char* param1, char* param2, char* param3);
Trama setStringTrama(char *string);
void shortToChars(short valor, char *cadena);
void setTramaString (Trama trama, int fd);
Trama TramaCreate (char type, char *header, char *data);
void freeTrama(Trama trama);

#endif