/*
Autors:
    Alba Mayol Lozano -->alba.mayol
    Kevin Eljarrat Ohayon --> kevin.eljarrat
    LaSalle - Sistemes Operatius
Data creació: 17/10/23
Data última modificació: 16/5/24
*/

#ifndef _GLOBAL_H_
#define _GLOBAL_H_

#define _GNU_SOURCE

#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include <errno.h>
#include <signal.h>
#include <stdint.h>
#include <sys/wait.h> 
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <limits.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/msg.h>
#include <sys/ipc.h>
#include "semaphore_v2.h"
#include "Strings.h"
#include "Commands.h"
#include "FilesCreation.h"
#include "Logica.h"
#include "Screen.h"



typedef struct {
	char* name;	
	char* ip;
	int port;
	int num_connections;
} Element;

void freeElement(Element* e);
void freePoolesArray(Element *array, int size);
Element pooleMinConnections(Element *poole_list, int poole_list_size);
void printListPooles(Element *poole_list, int poole_list_size);
int decreaseNumConnections(Element *poole_list, int poole_list_size, char* pooleName);
int erasePooleFromList(Element** poole_list, int* poole_list_size, char* pooleName);
void separaDataToElement(char* data, Element* e) ;

#endif