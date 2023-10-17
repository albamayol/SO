#include "Global.h"


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
            pathServerFile = read_until(fd, '\n');
            ipDiscovery = read_until(fd, '\n');
            puertoDiscovery = read_until(fd, '\n');
            ipServer = read_until(fd, '\n');
            puertoServer = read_until(fd, '\n');

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