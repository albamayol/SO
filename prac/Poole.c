#define _GNU_SOURCE

#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <strings.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <ctype.h>
#include <signal.h>
#include <stdint.h>
#include <sys/wait.h> 

#define printF(x) write(1, x, strlen(x))


char* read_until(int fd, char delimiter) {
    char *msg = NULL;
    char current;
    int i = 0;

    while(read(fd, &current, 1) > 0){
        if (i == 0) {
            msg = (char *) malloc(1);
        }

        if (current != delimiter) {
            msg[i] = current;
            msg = (char *) realloc(msg, ++i + 1);
        } 
        else {
            msg[i] = '\0';
            break;
        }
        printf("CHAR: %c\n", current);
    }  

    return msg;
}

void printString(char *cadena) {
    char *msg;
    asprintf(&msg, "%s\n", cadena);
    write(1, msg, strlen(msg));
    free(msg);
}

int main(int argc, char ** argv) {
    //char *msg = NULL;
    
    char *serverName = NULL;
	char *pathServerFile = NULL;
    char *ipDiscovery = NULL; 
    char *puertoDiscovery = NULL;
    char *ipServer = NULL; 
    char *puertoServer = NULL;

    if (argc != 2) {
        printF("ERROR. Number of arguments is not correct\n");
        exit(EXIT_FAILURE);
    } else {
        int fd = open(argv[1], O_RDONLY);
        if (fd == -1) {
            printF("ERROR. Could not open user's file\n");
            exit(EXIT_FAILURE);
        } else {
            serverName = read_until(fd, '\n');
            printString(serverName);
            pathServerFile = read_until(fd, '\n');
            printString(pathServerFile);
            ipDiscovery = read_until(fd, '\n');
            printString(ipDiscovery);
            puertoDiscovery = read_until(fd, '\n');
            printString(puertoDiscovery);
            ipServer = read_until(fd, '\n');
            printString(ipServer);
            puertoServer = read_until(fd, '\n');
            printString(puertoServer);


            free(serverName);
            serverName = NULL;
            free(pathServerFile);
            pathServerFile = NULL;
            free(ipDiscovery);
            ipDiscovery = NULL;
            free(puertoDiscovery);
            puertoDiscovery = NULL;
            free(ipServer);
            ipServer = NULL;
            free(puertoServer);
            puertoServer = NULL;

            close(fd);
        }
    }
    return 0;
}