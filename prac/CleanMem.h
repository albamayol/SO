/*
Autors:
    Alba Mayol Lozano -->alba.mayol
    Kevin Eljarrat Ohayon --> kevin.eljarrat
    LaSalle - Sistemes Operatius
Data creació: 23/05/24
Data última modificació: 23/05/24
*/

#ifndef _CLEANMEM_H_
#define _CLEANMEM_H_

#define _GNU_SOURCE

#include <string.h>
#include <strings.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

typedef struct {
    char *nombreCancion;	    
    char *nombrePlaylist;
    float porcentaje;   
    pthread_t thread_id;
} Descarga;

typedef struct {
    pthread_t thread;	
    char *nombreDescargaComando;
    int fd_bowman; 
} DescargaPoole;

typedef struct {
	pthread_t thread;	
	char* user_name;
    int fd;
    DescargaPoole *descargas;
    int numDescargas; // Num total de descargas por parte de un Bowman
} ThreadPoole;

typedef struct {
    char* nameplaylist;
    int numSongs; 
} InfoPlaylist;

void cleanThreadsPoole(ThreadPoole** threads, int numThreads);
void cleanThreadPoole(ThreadPoole* thread);
void cleanThreadsBowman(Descarga **descargas, int *numDescargas, int *maxDesc);
void cleanAllTheThreadsBowman(Descarga **descargas, int numDescargas);
void cleanInfoPlaylists(InfoPlaylist *infoPlaylists, int size);

#endif