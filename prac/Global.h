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
#include <limits.h>
#include <dirent.h>
//#include "semaphore_v2.h"

#define printF(x) write(1, x, strlen(x))

typedef struct {
	char* name;	
	char* ip;
	int port;
	int num_connections;
} Element;

typedef struct {
	pthread_t thread;	
	char* user_name;
    int fd;
} Thread;

typedef struct {
    int fdDiscovery;
    int fdPoole;
    struct sockaddr_in discovery_addr;
    struct sockaddr_in poole_addr;
    char *msg;
    char *input;
    char *upperInput;
    char *clienteNameAux;
    char *clienteName;
    char *pathClienteFile;
    char *ip;
    char *puerto;
    int bowmanConnected;
    Element pooleConnected;
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
    Thread *threads;
    int threads_array_size;
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
    Element *poole_list;
    pthread_mutex_t mutexList;
    int poole_list_size;
} dataDiscovery;

char* read_until(int fd, char delimiter);
char* read_until_string(char *string, char delimiter);
void cleanThreads(Thread** threads, int numThreads);
void cleanThread(Thread* thread);
void removeExtraSpaces(char *comanda);
char * to_upper(char * str);
int checkDownloadCommand(char * input);
char * verifyClientName(char * clienteNameAux);
void checkDownload(char *downloadPtr);
void separaDataToElement(char* data, Element* e);
Element pooleMinConnections(Element *poole_list, int poole_list_size);
void printListPooles(Element *poole_list, int poole_list_size);
int decreaseNumConnections(Element *poole_list, int poole_list_size, char* pooleName);
int erasePooleFromList(Element** poole_list, int* poole_list_size, char* pooleName);
char* convertIntToString(int num);
void freeElement(Element* e);
void freeString(char **string);
void freePoolesArray(Element *array, int size);
void createDirectory(char* directory);
char* readNumChars(char *string, int inicio, int final);
char* readUntilFromIndex(char *string, int *inicio, char delimiter, char *fina, char delimitadorFinal);
int songOrPlaylist(char *string);
void removeExtraSpaces(char *comanda);
char * to_upper(char * str);

#endif