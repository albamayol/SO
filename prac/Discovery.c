/*
Autores:
    Alba Mayol Lozano -->alba.mayol
    Kevin Eljarrat Ohayon --> kevin.eljarrat
    LaSalle - Sistemes Operatius
*/

#include "Trama.h"

dataDiscovery dDiscovery;

/*
@Finalitat: Inicialitza les variables a NULL o al valor inicial desitjat.
@Paràmetres: ---
@Retorn: ---
*/
void inicializarDataDiscovery() {
    dDiscovery.ipPoole = NULL;
	dDiscovery.portPoole = NULL;
    dDiscovery.ipBowman = NULL; 
    dDiscovery.portBowman = NULL;
    dDiscovery.poole_list = NULL;
    dDiscovery.poole_list_size = 0;
}

/*
@Finalitat: Gestiona la recepció de la signal (SIGINT) i allibera els recursos utilizats fins el moment
@Paràmetres: ---
@Retorn: ---
*/
void sig_func() {
    pthread_mutex_destroy(&dDiscovery.mutexList);
    if (dDiscovery.ipPoole != NULL) {
        freeString(&dDiscovery.ipPoole);
    }
    if (dDiscovery.portPoole != NULL) {
        freeString(&dDiscovery.portPoole);
    }
    if (dDiscovery.ipBowman != NULL) {
        freeString(&dDiscovery.ipBowman);
    }
    if (dDiscovery.portBowman != NULL) {
        freeString(&dDiscovery.portBowman);
    }
    
    freePoolesArray(dDiscovery.poole_list, dDiscovery.poole_list_size);

    close(dDiscovery.fdPoole);
    close(dDiscovery.fdBowman);

    exit(EXIT_SUCCESS);
}

/*
@Finalitat: Gestiona les connexions de Poole's, ja sigui per avisar de desonnexions (del propi Poole o d'algun dels seus Bowman's) i es guarda la informació del nou Poole a un array
@Paràmetres: int fd_poole: file descriptor del Poole connectat
@Retorn: ---
*/
void conexionPoole(int fd_poole) {
    char *buffer = NULL;
    TramaExtended tramaExtended = readTrama(fd_poole);

    if (strcmp(tramaExtended.trama.header, "BOWMAN_LOGOUT") == 0) {
        char* nameCleaned = NULL;
        nameCleaned = read_until_string(tramaExtended.trama.data, '~');

        if (decreaseNumConnections(dDiscovery.poole_list, dDiscovery.poole_list_size, nameCleaned)) {
            setTramaString(TramaCreate(0x06, "CONOK", "", 0), fd_poole);   
        } else {
            setTramaString(TramaCreate(0x06, "CONKO", "", 0), fd_poole);
        }
        freeString(&nameCleaned);
        freeTrama(&(tramaExtended.trama));
    } else if (strcmp(tramaExtended.trama.header, "POOLE_DISCONNECT") == 0) {
        char* nameCleaned = NULL;
        nameCleaned = read_until_string(tramaExtended.trama.data, '~');
        pthread_mutex_lock(&dDiscovery.mutexList); 
        int erasePooleResult = erasePooleFromList(&dDiscovery.poole_list, &dDiscovery.poole_list_size, nameCleaned);
        pthread_mutex_unlock(&dDiscovery.mutexList);   

        if (erasePooleResult) {
            setTramaString(TramaCreate(0x06, "CONOK", "", 0), fd_poole);   
        } else {
            setTramaString(TramaCreate(0x06, "CONKO", "", 0), fd_poole);
        }
        printListPooles(dDiscovery.poole_list, dDiscovery.poole_list_size);
        freeString(&nameCleaned);
        freeTrama(&(tramaExtended.trama));
    } else if (strcmp(tramaExtended.trama.header, "NEW_POOLE") == 0) {
        Element element;

        separaDataToElement(tramaExtended.trama.data, &element);
        freeTrama(&(tramaExtended.trama));

        asprintf(&buffer, "sizeArrayPooles: %d \n", dDiscovery.poole_list_size);
        printF(buffer);
        freeString(&buffer);

        pthread_mutex_lock(&dDiscovery.mutexList);
        dDiscovery.poole_list = (Element *)realloc(dDiscovery.poole_list, (dDiscovery.poole_list_size + 1) * sizeof(Element));
        
        dDiscovery.poole_list[dDiscovery.poole_list_size].name = strdup(element.name);
        dDiscovery.poole_list[dDiscovery.poole_list_size].ip = strdup(element.ip);
        dDiscovery.poole_list[dDiscovery.poole_list_size].port = element.port;
        dDiscovery.poole_list[dDiscovery.poole_list_size].num_connections = element.num_connections;
        dDiscovery.poole_list_size++;
        pthread_mutex_unlock(&dDiscovery.mutexList);

        freeElement(&element);

        printListPooles(dDiscovery.poole_list, dDiscovery.poole_list_size);

        setTramaString(TramaCreate(0x01, "CON_OK", "", 0), fd_poole);    
    }

    close(fd_poole);
}

/*
@Finalitat: Gestiona la connexió d'un Bowman, es busca el Poole amb el min de connexions i se li retorna la seva informació
@Paràmetres: int fd_bowman: file descriptor del Bowman connectat
@Retorn: ---
*/
void conexionBowman(int fd_bowman) {
    TramaExtended tramaExtended = readTrama(fd_bowman);
    freeTrama(&(tramaExtended.trama));

    pthread_mutex_lock(&dDiscovery.mutexList);
    Element e = pooleMinConnections(dDiscovery.poole_list, dDiscovery.poole_list_size); 
    pthread_mutex_unlock(&dDiscovery.mutexList);

    if (e.num_connections == -1) {
        setTramaString(TramaCreate(0x01, "CON_KO", "", 0), fd_bowman);
    } else {
        char* aux = NULL;
        char* portString = NULL;
        portString = convertIntToString(e.port);

        aux = createString3Params(e.name, e.ip, portString);
        freeElement(&e);
        freeString(&portString);
        
        setTramaString(TramaCreate(0x01, "CON_OK", aux, strlen(aux)), fd_bowman);
        freeString(&aux);
        printListPooles(dDiscovery.poole_list, dDiscovery.poole_list_size);
    }

    close(fd_bowman);
}

/*
@Finalitat: Accepta connexions amb Poole's
@Paràmetres: ----
@Retorn: ---
*/
void connect_Poole() {
    socklen_t pAddr = sizeof(dDiscovery.poole_addr);
    int fd_poole = accept(dDiscovery.fdPoole, (struct sockaddr *)&dDiscovery.poole_addr, &pAddr); 
    if (fd_poole < 0) { 
        perror("Error al aceptar la conexión de Poole");
        close(fd_poole);
        return;
    }
    conexionPoole(fd_poole);
}

/*
@Finalitat: Accepta connexions amb Bowman's
@Paràmetres: ----
@Retorn: ---
*/
void connect_Bowman() {
    socklen_t bAddr = sizeof(dDiscovery.bowman_addr);
    int fd_bowman = accept(dDiscovery.fdBowman, (struct sockaddr *)&dDiscovery.bowman_addr, &bAddr);
    if (fd_bowman < 0) {
        perror("Error al aceptar la conexión de Bowman");
        close(fd_bowman);
        return;
    }
    conexionBowman(fd_bowman);
}

/*
@Finalitat: Obre socket i espera noves connexions amb Poole's
@Paràmetres: ----
@Retorn: ---
*/
void startPooleListener() {

    dDiscovery.fdPoole = socket (AF_INET, SOCK_STREAM, IPPROTO_TCP);   
    if (dDiscovery.fdPoole < 0) {
        perror ("Error al crear el socket de Poole");
        exit (EXIT_FAILURE);
    } 

    bzero (&dDiscovery.poole_addr, sizeof (dDiscovery.poole_addr));
    dDiscovery.poole_addr.sin_family = AF_INET;
    dDiscovery.poole_addr.sin_port = htons (atoi(dDiscovery.portPoole));
    dDiscovery.poole_addr.sin_addr.s_addr = inet_addr(dDiscovery.ipPoole);

    if (bind (dDiscovery.fdPoole, (void *) &dDiscovery.poole_addr, sizeof (dDiscovery.poole_addr)) < 0) {
        perror ("Error al enlazar el socket de Poole");
        close(dDiscovery.fdPoole);
        exit (EXIT_FAILURE);
    }
    listen (dDiscovery.fdPoole, 20);
    
    while(1) {
        connect_Poole();
    }
}

/*
@Finalitat: Obre socket i espera noves connexions amb Bowman's
@Paràmetres: ----
@Retorn: ---
*/
void startBowmanListener() {
    dDiscovery.fdBowman = socket (AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (dDiscovery.fdBowman < 0) {
        perror ("Error al crear el socket de Bowman");
        sig_func();
    }

    bzero (&dDiscovery.bowman_addr, sizeof (dDiscovery.bowman_addr));
    dDiscovery.bowman_addr.sin_family = AF_INET;
    dDiscovery.bowman_addr.sin_port = htons (atoi(dDiscovery.portBowman));
    dDiscovery.bowman_addr.sin_addr.s_addr = inet_addr(dDiscovery.ipBowman);

    if (bind (dDiscovery.fdBowman, (void *) &dDiscovery.bowman_addr, sizeof (dDiscovery.bowman_addr)) < 0) {
        perror ("Error al enlazar el socket de Bowman");
        close(dDiscovery.fdBowman);
        sig_func();
    }
    listen (dDiscovery.fdBowman, 20);

    while(1) {
        connect_Bowman();
    }
}

/*
@Finalitat: Funció de thread per a les connexions de Bowman's
@Paràmetres: ----
@Retorn: ---
*/
static void *initial_thread_function_bowman() { 
    startBowmanListener();
    return NULL;
}

/*
@Finalitat: Implementar el main del programa, dona pas a la creació dels dos sockets (un per les connexions de Poole's i l'altra per a les de Bowman's)
@Paràmetres: int argc: número d'arguments de programa, char** argv: array amb els arguments del programa
@Retorn: int: 0 en cas que el programa hagi finalitzat exitosament.
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

            close(fd);
            
            pthread_t initial_thread_bowman;
            if (pthread_create(&initial_thread_bowman, NULL, initial_thread_function_bowman, NULL) != 0) {
                perror("Error al crear el thread inicial para Bowman");
            }

            startPooleListener();

            sig_func();
        }
    }
    return 0;
}