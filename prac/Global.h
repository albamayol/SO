#ifndef _GLOBAL_H_
#define _GLOBAL_H_

#define _GNU_SOURCE

#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <strings.h>
/*
Autores:
    Alba Mayol Lozano -->alba.mayol
    Kevin Eljarrat Ohayon --> kevin.eljarrat
*/

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <ctype.h>
#include <signal.h>
#include <stdint.h>
#include <sys/wait.h> 

#define printF(x) write(1, x, strlen(x))

typedef struct {
    char *msg;
    char *input;
    char *upperInput;
    char *clienteNameAux;
    char *clienteName;
    char *pathClienteFile;
    char *ip;
    char *puerto;
} dataBowman;

typedef struct {
    char *serverName;
	char *pathServerFile;
    char *ipDiscovery; 
    char *puertoDiscovery;
    char *ipServer; 
    char *puertoServer;
} dataPoole;

char* read_until(int fd, char delimiter);

#endif