/*
Autores:
    Alba Mayol Lozano -->alba.mayol
    Kevin Eljarrat Ohayon --> kevin.eljarrat
*/

#ifndef _GLOBAL_H_
#define _GLOBAL_H_

#define _GNU_SOURCE

#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <strings.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <errno.h>
#include <ctype.h>
#include <signal.h>
#include <stdint.h>
#include <sys/wait.h> 
#include <sys/socket.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <pthread.h>
#include <arpa/inet.h>
#include "linkedlist.h"
//#include "Trama.h"


#define printF(x) write(1, x, strlen(x))

//12 --> ~
//short header_length
//read(fd, header_length, sizeof(char)*2)

/*
typedef struct {
    char type;
    short header_length; //12
    char *header;
    char *data;
} Trama;
*/

typedef struct {
    int fdDiscovery;
    struct sockaddr_in discovery_addr;
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
    int fdPooleServer;
    int fdPooleClient;
    struct sockaddr_in discovery_addr;
    struct sockaddr_in poole_addr;
    char *serverName;
	char *pathServerFile;
    char *ipDiscovery; 
    char *puertoDiscovery;
    char *ipServer; 
    char *puertoServer;
    char *msg;
} dataPoole;

typedef struct {
    int fdPoole;
    int fdBowman;
    struct sockaddr_in poole_addr;
    struct sockaddr_in bowman_addr;
    char *ipPoole;
    char *ipBowman; 
    char *portPoole;
    char *portBowman;
    LinkedList poole_list;
    LinkedList bowman_list;
} dataDiscovery;

char* read_until(int fd, char delimiter);
char* read_until_string(char *string, char delimiter);
void separaDataToElement(char* data, Element* e);
Element pooleMinConnections(LinkedList * list);
void freeElement(Element* e);

#endif