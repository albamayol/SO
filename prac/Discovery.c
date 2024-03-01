/*
Autores:
    Alba Mayol Lozano -->alba.mayol
    Kevin Eljarrat Ohayon --> kevin.eljarrat
*/

#include "Trama.h"

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
    dDiscovery.poole_list = NULL;
    dDiscovery.poole_list_size = 0;
}

/*
@Finalitat: Manejar la recepción de la signal (SIGINT) y liberar los recursos utilizados hasta el momento.
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

    exit(EXIT_FAILURE);
}

void conexionPoole(int fd_poole) {
    char *buffer = NULL;
    TramaExtended tramaExtended = readTrama(fd_poole);

    if (strcmp(tramaExtended.trama.header, "BOWMAN_LOGOUT") == 0) {
        char* nameCleaned = NULL;
        nameCleaned = read_until_string(tramaExtended.trama.data, '~');
        printF("namePooleToDecreaseConnections: ");
        printF(nameCleaned);
        printF("\n");
        if (decreaseNumConnections(dDiscovery.poole_list, dDiscovery.poole_list_size, nameCleaned)) {
            setTramaString(TramaCreate(0x06, "CONOK", "", 0), fd_poole);   
        } else {
            setTramaString(TramaCreate(0x06, "CONKO", "", 0), fd_poole);
        }
        freeString(&nameCleaned);
        freeTrama(&(tramaExtended.trama));
    } else if (strcmp(tramaExtended.trama.header, "POOLE_DISCONNECT") == 0) {
        printListPooles(dDiscovery.poole_list, dDiscovery.poole_list_size);
        char* nameCleaned = NULL;
        nameCleaned = read_until_string(tramaExtended.trama.data, '~');
        pthread_mutex_lock(&dDiscovery.mutexList); 
        int erasePooleResult = erasePooleFromList(&dDiscovery.poole_list, &dDiscovery.poole_list_size, nameCleaned);
        pthread_mutex_unlock(&dDiscovery.mutexList);   

        if (erasePooleResult) {
            setTramaString(TramaCreate(0x06, "CONOK", "", 0), fd_poole);   
            asprintf(&buffer, "sizeArrayPoolesWhenPooleDisconnects: %d \n", dDiscovery.poole_list_size);
            printF(buffer);
            freeString(&buffer);
        } else {
            setTramaString(TramaCreate(0x06, "CONKO", "", 0), fd_poole);
        }
        freeString(&nameCleaned);
        freeTrama(&(tramaExtended.trama));
    } else {
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

        asprintf(&buffer, "sizeArrayPoolesUpdated: %d \n", dDiscovery.poole_list_size);
        printF(buffer);
        freeString(&buffer);

        freeElement(&element);

        printListPooles(dDiscovery.poole_list, dDiscovery.poole_list_size);

        setTramaString(TramaCreate(0x01, "CON_OK", "", 0), fd_poole);    
    }

    close(fd_poole);
}


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

void connect_Poole() {
    socklen_t pAddr = sizeof(dDiscovery.poole_addr);
    int fd_poole = accept(dDiscovery.fdPoole, (struct sockaddr *)&dDiscovery.poole_addr, &pAddr); 
    if (fd_poole < 0) { 
        //setTramaString(TramaCreate(0x01, "CON_KO", ""), fd_poole);    //TODO REVISAR DONDE VA ESTA TRAMA CON_KO!!!
        perror("Error al aceptar la conexión de Poole");
        close(fd_poole);
        return;
    }

    conexionPoole(fd_poole);
}

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
    printf("after bind\n");
    listen (dDiscovery.fdPoole, 20);
    
    while (1) {
        connect_Poole();
    }
}

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

static void *initial_thread_function_bowman() { 
    startBowmanListener();
    return NULL;
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