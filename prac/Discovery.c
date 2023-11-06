/*
Autores:
    Alba Mayol Lozano -->alba.mayol
    Kevin Eljarrat Ohayon --> kevin.eljarrat
*/

#include "Global.h"

dataDiscovery dDiscovery;

/*
@Finalitat: Inicializar las variables a NULL.
@Paràmetres: ---
@Retorn: ---
*/
void inicializarDataDiscovery() {
    dDiscovery.ipPoole = NULL;
	dDiscovery.portPoole = NULL;
    dDiscovery.ipBowman = NULL; 
    dDiscovery.portBowman = NULL;
}

/*
@Finalitat: Manejar la recepción de la signal (SIGINT) y liberar los recursos utilizados hasta el momento.
@Paràmetres: ---
@Retorn: ---
*/
void sig_func() {
    if (dDiscovery.ipPoole != NULL) {
        free(dDiscovery.ipPoole);
        dDiscovery.ipPoole = NULL;
    }
    if (dDiscovery.portPoole != NULL) {
        free(dDiscovery.portPoole);
        dDiscovery.portPoole = NULL;
    }
    if (dDiscovery.ipBowman != NULL) {
        free(dDiscovery.ipBowman);
        dDiscovery.ipBowman = NULL;
    }
    if (dDiscovery.portBowman != NULL) {
        free(dDiscovery.portBowman);
        dDiscovery.portBowman = NULL;
    }
    exit(EXIT_FAILURE);
}

void *clientHandler(void *arg) {
    int clientSocket = *((int *)arg);
    
    // Aquí puedes leer datos del cliente y procesarlos
    
    close(clientSocket);
    pthread_exit(NULL);
}

void startPooleListener() {
    dDiscovery.fdPoole = socket (AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (dDiscovery.fdPoole < 0) {
        perror ("Error al crear el socket de Poole");
        exit (EXIT_FAILURE);
    }

    // Specify the adress and port of the socket
    // We'll admit connexions to any IP of our machine in the specified port
    bzero (&dDiscovery.poole_addr, sizeof (dDiscovery.poole_addr));
    dDiscovery.poole_addr.sin_family = AF_INET;
    dDiscovery.poole_addr.sin_port = htons (atoi(dDiscovery.portPoole));

    if (inet_pton(AF_INET, dDiscovery.ipPoole, &dDiscovery.poole_addr.sin_port) < 0) {
        perror("Error al convertir la dirección IP");
        exit(EXIT_FAILURE);
    }

    // When executing bind, we should add a cast:
    // bind waits for a struct sockaddr* and we are passing a struct sockaddr_in*
    if (bind (dDiscovery.fdPoole, (void *) &dDiscovery.poole_addr, sizeof (dDiscovery.poole_addr)) < 0) {
        perror ("Error al enlazar el socket de Poole");
        exit (EXIT_FAILURE);
    }

    // We now open the port (20 backlog queue, typical value)
    listen (dDiscovery.fdPoole, 20);


    // Procesamos las peticiones de Poole
    while (1) {
        socklen_t pAddr = sizeof(dDiscovery.poole_addr);
        int new_fd = accept(dDiscovery.fdPoole, (struct sockaddr *)&dDiscovery.poole_addr, &pAddr);
        if (new_fd < 0) {
            perror("Error al aceptar la conexión de Poole");
            continue;
        }

        pthread_t thread;
        if (pthread_create(&thread, NULL, clientHandler, &new_fd) != 0) {
            perror("Error al crear el hilo para el cliente");
        }
    }
}

void startBowmanListener() {
    dDiscovery.fdBowman = socket (AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (dDiscovery.fdBowman < 0) {
        perror ("Error al crear el socket de Bowman");
        exit (EXIT_FAILURE);
    }

    // Specify the adress and port of the socket
    // We'll admit connexions to any IP of our machine in the specified port
    bzero (&dDiscovery.bowman_addr, sizeof (dDiscovery.bowman_addr));
    dDiscovery.bowman_addr.sin_family = AF_INET;
    dDiscovery.bowman_addr.sin_port = htons (atoi(dDiscovery.portBowman));

    if (inet_pton(AF_INET, dDiscovery.ipBowman, &dDiscovery.bowman_addr.sin_port) < 0) {
        perror("Error al convertir la dirección IP");
        exit(EXIT_FAILURE);
    }

    // When executing bind, we should add a cast:
    // bind waits for a struct sockaddr* and we are passing a struct sockaddr_in*
    if (bind (dDiscovery.fdBowman, (void *) &dDiscovery.bowman_addr, sizeof (dDiscovery.bowman_addr)) < 0) {
        perror ("Error al enlazar el socket de Bowman");
        exit (EXIT_FAILURE);
    }

    // We now open the port (20 backlog queue, typical value)
    listen (dDiscovery.fdBowman, 20);
}

/*
@Finalitat: Implementar el main del programa.
@Paràmetres: ---
@Retorn: int: Devuelve 0 en caso de que el programa haya finalizado exitosamente.
*/
int main(int argc, char ** argv) {
    
    inicializarDataDiscovery();
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
            dDiscovery.ipPoole = read_until(fd, '\n');
            dDiscovery.portPoole = read_until(fd, '\n');
            dDiscovery.ipBowman = read_until(fd, '\n');
            dDiscovery.portBowman = read_until(fd, '\n');

            free(dDiscovery.ipPoole);
            dDiscovery.ipPoole = NULL;
            free(dDiscovery.portPoole);
            dDiscovery.portPoole = NULL;
            free(dDiscovery.ipBowman);
            dDiscovery.ipBowman = NULL;
            free(dDiscovery.portBowman);
            dDiscovery.portBowman = NULL;

            close(fd);
            
            startPooleListener();
            startBowmanListener();
            

            
        }
    }
    return 0;
}