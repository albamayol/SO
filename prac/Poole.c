/*
Autores:
    Alba Mayol Lozano -->alba.mayol
    Kevin Eljarrat Ohayon --> kevin.eljarrat
*/

#include "Global.h"
#include "Trama.h"

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
    dPoole.msg = NULL;
}

/*
@Finalitat: Manejar la recepción de la signal (SIGINT) y liberar los recursos utilizados hasta el momento.
@Paràmetres: ---
@Retorn: ---
*/
void sig_func() {
    if (dPoole.serverName != NULL) {
        freeString(dPoole.serverName);
    }
    if (dPoole.pathServerFile != NULL) {
        freeString(dPoole.pathServerFile);
    }
    if (dPoole.ipDiscovery != NULL) {
        freeString(dPoole.ipDiscovery);
    }
    if (dPoole.puertoDiscovery != NULL) {
        freeString(dPoole.puertoDiscovery);
    }
    if (dPoole.ipServer != NULL) {
        freeString(dPoole.ipServer);
    }
    if (dPoole.puertoServer != NULL) {
        freeString(dPoole.puertoServer);
    }
    if (dPoole.msg != NULL) {
        freeString(dPoole.msg);
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
    asprintf(&dPoole.msg, "Server - %s\n", dPoole.serverName);
    printF(dPoole.msg);
    free(dPoole.msg);
    

    asprintf(&dPoole.msg, "Server Directory - %s\n", dPoole.pathServerFile);
    printF(dPoole.msg);
    free(dPoole.msg);
    

    asprintf(&dPoole.msg, "IP Discovery - %s\n", dPoole.ipDiscovery);
    printF(dPoole.msg);
    free(dPoole.msg);
    

    asprintf(&dPoole.msg, "Port Server - %s\n\n", dPoole.puertoServer);
    printF(dPoole.msg);
    free(dPoole.msg);

    asprintf(&dPoole.msg, "IP Server - %s\n", dPoole.ipServer);
    printF(dPoole.msg);
    free(dPoole.msg);

    asprintf(&dPoole.msg, "Port Server - %s\n\n", dPoole.puertoServer);
    printF(dPoole.msg);
    free(dPoole.msg);

    dPoole.msg = NULL;

}

void establishDiscoveryConnection() {
    dPoole.fdPooleClient = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (dPoole.fdPooleClient < 0) {
        perror ("Error al crear el socket de Discovery");
        close(dPoole.fdPooleClient);
        sig_func();
    }

    bzero (&dPoole.discovery_addr, sizeof (dPoole.discovery_addr));
    dPoole.discovery_addr.sin_family = AF_INET;
    dPoole.discovery_addr.sin_port = htons(atoi(dPoole.puertoDiscovery)); 

    if (inet_pton(AF_INET, dPoole.ipDiscovery, &dPoole.discovery_addr.sin_addr) < 0) {
        perror("Error al convertir la dirección IP");
        close(dPoole.fdPooleClient);
        sig_func();
    }

    if (connect(dPoole.fdPooleClient, (struct sockaddr*)&dPoole.discovery_addr, sizeof(dPoole.discovery_addr)) < 0) {
        perror("Error al conectar a Discovery");
        close(dPoole.fdPooleClient);
        sig_func();
    }

    //TRANSMISIONES POOLE->DISCOVERY
    
    char* aux = NULL;
    aux = createString3Params(dPoole.serverName, dPoole.ipServer, dPoole.puertoServer);
    setTramaString(TramaCreate(0x01, NEW_POOLE, anadirClaudators(aux)), dPoole.fdPooleClient);
    freeString(aux);
}

void waitingForRequests() {
    dPoole.fdPooleServer = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (dPoole.fdPooleServer < 0) {
        perror ("Error al crear el socket de Bowman");
        close(dPoole.fdPooleServer);
        sig_func();
    }

    bzero (&dPoole.poole_addr, sizeof (dPoole.poole_addr));
    dPoole.poole_addr.sin_family = AF_INET;
    dPoole.poole_addr.sin_port = htons(atoi(dPoole.puertoServer)); 
    
    if (inet_pton(AF_INET, dPoole.ipServer, &dPoole.poole_addr.sin_addr) < 0) {
        perror("Error al convertir la dirección IP");
        close(dPoole.fdPooleServer);
        sig_func();
    }

    if (bind(dPoole.fdPooleServer, (struct sockaddr*)&dPoole.poole_addr, sizeof(dPoole.poole_addr)) < 0) {
        perror("Error al enlazar el socket de Poole");
        close(dPoole.fdPooleServer);
        sig_func();
    }

    listen(dPoole.fdPooleServer, 20); // Esperar conexiones entrantes de Bowman
    //TRANSMISIONES POOLE<->BOWMAN



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

            printInfoFile();

            freeString(dPoole.serverName);
            freeString(dPoole.pathServerFile);
            freeString(dPoole.ipDiscovery);
            freeString(dPoole.puertoDiscovery);
            freeString(dPoole.ipServer);
            freeString(dPoole.puertoServer);

            close(fd);

            establishDiscoveryConnection();
            //waitingForRequests();
        }
    }
    return 0;
}