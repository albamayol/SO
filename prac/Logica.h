/*
Autors:
    Alba Mayol Lozano -->alba.mayol
    Kevin Eljarrat Ohayon --> kevin.eljarrat
    LaSalle - Sistemes Operatius
Data creació: 23/05/24
Data última modificació: 23/05/24
*/

#ifndef _LOGICA_H_
#define _LOGICA_H_

#define _GNU_SOURCE

#include <string.h>
#include <strings.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <math.h>
#include <dirent.h>
#include "semaphore_v2.h"
#include <sys/msg.h>
#include <sys/ipc.h>

int min(size_t a, size_t b);
int getRandomID();
int searchSong(char *pathSongPlaylist, int *fileSize);
int searchPlaylist(char *pathSongPlaylist);
int contarArchivosRegulares(const char *path);
void listSongs(const char *path, char **fileNames, int *totalSongs);
void listPlaylists(const char *path, char **fileNames, int *totalSongs);
void checkPooleConnection(int *bowmanConnected, char* clienteName);
void creacionMsgQueues(int *msgQueueDescargas, int* msgQueuePetitions);
#endif