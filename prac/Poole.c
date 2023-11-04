/*
Autores:
    Alba Mayol Lozano -->alba.mayol
    Kevin Eljarrat Ohayon --> kevin.eljarrat
*/

#include "Global.h"

dataPoole dDiscovery;

/*
@Finalitat: Inicializar las variables a NULL.
@Paràmetres: ---
@Retorn: ---
*/
void inicializarDataPoole() {
    dDiscovery.serverName = NULL;
	dDiscovery.pathServerFile = NULL;
    dDiscovery.ipDiscovery = NULL; 
    dDiscovery.puertoDiscovery = NULL;
    dDiscovery.ipServer = NULL; 
    dDiscovery.puertoServer = NULL;
    dDiscovery.msg = NULL;
}

/*
@Finalitat: Manejar la recepción de la signal (SIGINT) y liberar los recursos utilizados hasta el momento.
@Paràmetres: ---
@Retorn: ---
*/
void sig_func() {
    if (dDiscovery.serverName != NULL) {
        free(dDiscovery.serverName);
        dDiscovery.serverName = NULL;
    }
    if (dDiscovery.pathServerFile != NULL) {
        free(dDiscovery.pathServerFile);
        dDiscovery.pathServerFile = NULL;
    }
    if (dDiscovery.ipDiscovery != NULL) {
        free(dDiscovery.ipDiscovery);
        dDiscovery.ipDiscovery = NULL;
    }
    if (dDiscovery.puertoDiscovery != NULL) {
        free(dDiscovery.puertoDiscovery);
        dDiscovery.puertoDiscovery = NULL;
    }
    if (dDiscovery.ipServer != NULL) {
        free(dDiscovery.ipServer);
        dDiscovery.ipServer = NULL;
    }
    if (dDiscovery.puertoServer != NULL) {
        free(dDiscovery.puertoServer);
        dDiscovery.puertoServer = NULL;
    }
    if (dDiscovery.msg != NULL) {
        free(dDiscovery.msg);
        dDiscovery.msg = NULL;
    }
    exit(EXIT_FAILURE);
}

/*
@Finalitat: Printa la información leída de configPoole
@Paràmetres: ---
@Retorn: ---
*/
void printInfoFile() {
    
    printF("\nFile read correctly:\n");
    asprintf(&dDiscovery.msg, "Server - %s\n", dDiscovery.serverName);
    printF(dDiscovery.msg);
    free(dDiscovery.msg);
    

    asprintf(&dDiscovery.msg, "Server Directory - %s\n", dDiscovery.pathServerFile);
    printF(dDiscovery.msg);
    free(dDiscovery.msg);
    

    asprintf(&dDiscovery.msg, "IP Discovery - %s\n", dDiscovery.ipDiscovery);
    printF(dDiscovery.msg);
    free(dDiscovery.msg);
    

    asprintf(&dDiscovery.msg, "Port Server - %s\n\n", dDiscovery.puertoServer);
    printF(dDiscovery.msg);
    free(dDiscovery.msg);

    asprintf(&dDiscovery.msg, "IP Server - %s\n", dDiscovery.ipServer);
    printF(dDiscovery.msg);
    free(dDiscovery.msg);

    asprintf(&dDiscovery.msg, "Port Server - %s\n\n", dDiscovery.puertoServer);
    printF(dDiscovery.msg);
    free(dDiscovery.msg);

    dDiscovery.msg = NULL;

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
            dDiscovery.serverName = read_until(fd, '\n');
            dDiscovery.pathServerFile = read_until(fd, '\n');
            dDiscovery.ipDiscovery = read_until(fd, '\n');
            dDiscovery.puertoDiscovery = read_until(fd, '\n');
            dDiscovery.ipServer = read_until(fd, '\n');
            dDiscovery.puertoServer = read_until(fd, '\n');

            printInfoFile();

            free(dDiscovery.serverName);
            dDiscovery.serverName = NULL;
            free(dDiscovery.pathServerFile);
            dDiscovery.pathServerFile = NULL;
            free(dDiscovery.ipDiscovery);
            dDiscovery.ipDiscovery = NULL;
            free(dDiscovery.puertoDiscovery);
            dDiscovery.puertoDiscovery = NULL;
            free(dDiscovery.ipServer);
            dDiscovery.ipServer = NULL;
            free(dDiscovery.puertoServer);
            dDiscovery.puertoServer = NULL;

            close(fd);
        }
    }
    return 0;
}