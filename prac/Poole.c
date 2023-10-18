/*
Autores:
    Alba Mayol Lozano -->alba.mayol
    Kevin Eljarrat Ohayon --> kevin.eljarrat
*/

#include "Global.h"

dataPoole dPoole;

/*
@Finalitat: Inicializar las variables a NULL.
@Paràmetres: ---
@Retorn: ---
*/
void inicializarDataPoole() {
    dPoole.serverName = NULL;
	dPoole.pathServerFile = NULL;
    dPoole.ipDiscovery = NULL; 
    dPoole.puertoDiscovery = NULL;
    dPoole.ipServer = NULL; 
    dPoole.puertoServer = NULL;
}

/*
@Finalitat: Manejar la recepción de la signal (SIGINT) y liberar los recursos utilizados hasta el momento.
@Paràmetres: ---
@Retorn: ---
*/
void sig_func() {
    if (dPoole.serverName != NULL) {
        free(dPoole.serverName);
        dPoole.serverName = NULL;
    }
    if (dPoole.pathServerFile != NULL) {
        free(dPoole.pathServerFile);
        dPoole.pathServerFile = NULL;
    }
    if (dPoole.ipDiscovery != NULL) {
        free(dPoole.ipDiscovery);
        dPoole.ipDiscovery = NULL;
    }
    if (dPoole.puertoDiscovery != NULL) {
        free(dPoole.puertoDiscovery);
        dPoole.puertoDiscovery = NULL;
    }
    if (dPoole.ipServer != NULL) {
        free(dPoole.ipServer);
        dPoole.ipServer = NULL;
    }
    if (dPoole.puertoServer != NULL) {
        free(dPoole.puertoServer);
        dPoole.puertoServer = NULL;
    }
    exit(EXIT_FAILURE);
}

/*
@Finalitat: Implementar el main del programa.
@Paràmetres: ---
@Retorn: int: Devuelve 0 en caso de que el programa haya finalizado exitosamente.
*/
int main(int argc, char ** argv) {
    
    inicializarDataPoole();
    signal(SIGINT, sig_func);
    

    if (argc != 2) {
        printF("ERROR. Number of arguments is not correct\n");
        exit(EXIT_FAILURE);
    } else {
        int fd = open(argv[1], O_RDONLY);
        if (fd == -1) {
            printF("ERROR. Could not open user's file\n");
            exit(EXIT_FAILURE);
        } else {
            dPoole.serverName = read_until(fd, '\n');
            dPoole.pathServerFile = read_until(fd, '\n');
            dPoole.ipDiscovery = read_until(fd, '\n');
            dPoole.puertoDiscovery = read_until(fd, '\n');
            dPoole.ipServer = read_until(fd, '\n');
            dPoole.puertoServer = read_until(fd, '\n');

            free(dPoole.serverName);
            dPoole.serverName = NULL;
            free(dPoole.pathServerFile);
            dPoole.pathServerFile = NULL;
            free(dPoole.ipDiscovery);
            dPoole.ipDiscovery = NULL;
            free(dPoole.puertoDiscovery);
            dPoole.puertoDiscovery = NULL;
            free(dPoole.ipServer);
            dPoole.ipServer = NULL;
            free(dPoole.puertoServer);
            dPoole.puertoServer = NULL;

            close(fd);
        }
    }
    return 0;
}