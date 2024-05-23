/*
Autors:
    Alba Mayol Lozano -->alba.mayol
    Kevin Eljarrat Ohayon --> kevin.eljarrat
    LaSalle - Sistemes Operatius
Data creació: 23/05/24
Data última modificació: 23/05/24
*/

#ifndef _COMMANDS_H_
#define _COMMANDS_H_

#define _GNU_SOURCE

#include <string.h>
#include <strings.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int checkDownloadCommand(char * input);
void checkDownload(char *downloadPtr);
int songOrPlaylist(char *string);
char * resultMd5sumComand(char *pathName);

#endif