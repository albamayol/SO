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
    
    LINKEDLIST_destroy(&dDiscovery.poole_list); //el destroy itera por toda la list haciendo free's de los elementos
    LINKEDLIST_destroy(&dDiscovery.bowman_list);

    /* HACER EN EL FUTURO:
    Element client;

    LINKEDLIST_goToHead(&connections);
		
	while(!LINKEDLIST_isAtEnd(connections)) {
        client = LINKEDLIST_get(&connections);
    	kill(client.signalID, SIGINT);
		LINKEDLIST_next(&connections);
    }
	LINKEDLIST_destroy(&connections);*/
    exit(EXIT_FAILURE);
}

static void *thread_function_poole(void *fd) {
    int fd_poole = *((int *)fd);
    //TODO añadir conexion poole a la lista

    Element element;
    //add element as the last one
    while(!LINKEDLIST_isAtEnd(poole_list)) {
        LINKEDLIST_next(&poole_list);
    }
    LINKEDLIST_add(&poole_list, element);
    
    
    close(fd_poole);
    pthread_exit(NULL);
}

static void *thread_function_bowman(void *fd) {
    int fd_bowman = *((int *)fd);
    
    
    
    close(fd_bowman);
    pthread_exit(NULL);
}

void connect_Poole() {
    socklen_t pAddr = sizeof(dDiscovery.poole_addr);
    int fd_poole = accept(dDiscovery.fdPoole, (struct sockaddr *)&dDiscovery.poole_addr, &pAddr);
    if (fd_poole < 0) {
        perror("Error al aceptar la conexión de Poole");
        continue;
    }

    pthread_t thread_poole;
    if (pthread_create(&thread_poole, NULL, thread_function_poole, &fd_poole) != 0) {
        perror("Error al crear el thread para Poole");
    }
}

void connect_Bowman() {
    socklen_t bAddr = sizeof(dDiscovery.bowman_addr);
    int fd_bowman = accept(dDiscovery.fdBowman, (struct sockaddr *)&dDiscovery.bowman_addr, &bAddr);
    if (fd_bowman < 0) {
        perror("Error al aceptar la conexión de Poole");
        continue;
    }

    pthread_t thread_bowman;
    if (pthread_create(&thread_bowman, NULL, thread_function_bowman, &fd_bowman) != 0) {
        perror("Error al crear el thread para Poole");
    }
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


    // Procesamos las peticiones de Poole's
    while (1) {
        connect_Poole();
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

    //procesamos las peticiones de Bowman's
    while(1) {
        connect_Bowman();
    }
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
        dDiscovery.poole_list = LINKEDLIST_create();
        dDiscovery.bowman_list = LINKEDLIST_create();

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