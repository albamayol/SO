/*
Autors:
    Alba Mayol Lozano -->alba.mayol
    Kevin Eljarrat Ohayon --> kevin.eljarrat
    LaSalle - Sistemes Operatius
Data creació: 23/05/24
Data última modificació: 23/05/24
*/

#ifndef _SCREEN_H_
#define _SCREEN_H_

#define _GNU_SOURCE

#include <string.h>
#include <strings.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <pthread.h>
#include "ConfigurationBowman.h"
#include "ConfigurationPoole.h"
#include "ConfigurationDiscovery.h"

typedef struct {
    char* nameplaylist;
    int numSongs; 
} InfoPlaylist;

void initScreen();
void printF(char* x);
void destroyMutexScreen();
void printInfoFileBowman();
void printInfoFile();
void printarSongs(int numCanciones, char ***canciones);
void printarPlaylists(int numListas, char ***listas, int *numCancionesPorLista, InfoPlaylist **arrayInfo, int* numInfoPlaylists);
#endif