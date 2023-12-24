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

    //cancel y despues join

    exit(EXIT_FAILURE);
}

void conexionPoole(int fd_poole) {
    char *buffer = NULL;
    Trama trama = readTrama(fd_poole);
    if (strcmp(trama.header, "BOWMAN_LOGOUT") == 0) {
        //LOGOUT --> trama.data contiene el nombre del poole donde ha ocurrido un logout
        if (decreaseNumConnections(dDiscovery.poole_list, dDiscovery.poole_list_size, trama.data)) {
            printF(trama.header);
            printF(trama.data);
            setTramaString(TramaCreate(0x06, "CONOK", ""), fd_poole);   
        } else {
            setTramaString(TramaCreate(0x06, "CONKO", ""), fd_poole);
        }
        freeTrama(&trama);

    } else if (strcmp(trama.header, "POOLE_DISCONNECT") == 0) {
        printListPooles(dDiscovery.poole_list, dDiscovery.poole_list_size);

        pthread_mutex_lock(&dDiscovery.mutexList);  //LOCK
        int erasePooleResult = erasePooleFromList(&dDiscovery.poole_list, &dDiscovery.poole_list_size, trama.data)
        pthread_mutex_unlock(&dDiscovery.mutexList);    //UNLOCK

        if (erasePooleResult) {
            setTramaString(TramaCreate(0x06, "CONOK", ""), fd_poole);   
            asprintf(&buffer, "sizeArrayPoolesWhenPooleDisconnects: %d \n", dDiscovery.poole_list_size);
            printF(buffer);
            freeString(&buffer);
        } else {
            setTramaString(TramaCreate(0x06, "CONKO", ""), fd_poole);
        }
        freeTrama(&trama);
    } else {
        Element element;
        
        write(1, trama.header, strlen(trama.header));
        write(1, "\n", 1);
        write(1, trama.data, strlen(trama.data));
        separaDataToElement(trama.data, &element);
        freeTrama(&trama);
        
        write(1, "ELEMENT:\n", strlen("ELEMENT:\n"));
        write(1, element.name, strlen(element.name));
        write(1, "\n", 1);
        write(1, element.ip, strlen(element.ip));
        write(1, "\n", 1);
        char *msg = NULL;
        asprintf(&msg, "port: %d\n", element.port);
        printF(msg);
        freeString(&msg);

        //add element as the last one    
        asprintf(&buffer, "sizeArrayPooles: %d \n", dDiscovery.poole_list_size);
        printF(buffer);
        freeString(&buffer);

        //lock
        pthread_mutex_lock(&dDiscovery.mutexList);
        dDiscovery.poole_list = (Element *)realloc(dDiscovery.poole_list, (dDiscovery.poole_list_size + 1) * sizeof(Element));
        
        dDiscovery.poole_list[dDiscovery.poole_list_size].name = strdup(element.name);
        dDiscovery.poole_list[dDiscovery.poole_list_size].ip = strdup(element.ip);
        dDiscovery.poole_list[dDiscovery.poole_list_size].port = element.port;
        dDiscovery.poole_list[dDiscovery.poole_list_size].num_connections = element.num_connections;
        dDiscovery.poole_list_size++;
        pthread_mutex_unlock(&dDiscovery.mutexList);
        //unlock

        asprintf(&buffer, "sizeArrayPoolesUpdated: %d \n", dDiscovery.poole_list_size);
        printF(buffer);
        freeString(&buffer);

        freeElement(&element);

        printListPooles(dDiscovery.poole_list, dDiscovery.poole_list_size);

        setTramaString(TramaCreate(0x01, "CON_OK", ""), fd_poole);    
    }

    close(fd_poole);
}


void conexionBowman(int fd_bowman) {
    Trama trama = readTrama(fd_bowman);
    freeTrama(&trama);

    /*SE PUEDE DAR EL CASO QUE UN POOLE SE ESTE CONECTANDO/DESCONECTANDO (MODIFICAN LA LISTA) Y QUE A SU VEZ SE CONECTE UN BOWMAN(PIDA LA INFO DEL POOLE CON MINIMO DE CONEXIONES)*/
    //lock
    pthread_mutex_lock(&dDiscovery.mutexList);
    Element e = pooleMinConnections(dDiscovery.poole_list, dDiscovery.poole_list_size); // Enviar trama con servername, ip y port del Poole
    //unlock
    pthread_mutex_unlock(&dDiscovery.mutexList);

    if (e.num_connections == -1) {
        //NO HAY POOLE'S CONECTADOS! NO PODEMOS REDIRIGIR EL BOWMAN A NINGUN POOLE --> ENVIAMOS TRAMA CON_KO!!!
        setTramaString(TramaCreate(0x01, "CON_KO", ""), fd_bowman);
    } else {
        char* aux = NULL;
        char* portString = NULL;
        portString = convertIntToString(e.port);

        aux = createString3Params(e.name, e.ip, portString);
        freeElement(&e);
        freeString(&portString);
        
        setTramaString(TramaCreate(0x01, "CON_OK", aux), fd_bowman);
        freeString(&aux);
        printListPooles(dDiscovery.poole_list, dDiscovery.poole_list_size);
    }

    close(fd_bowman);
}

void connect_Poole() {
    socklen_t pAddr = sizeof(dDiscovery.poole_addr);
    int fd_poole = accept(dDiscovery.fdPoole, (struct sockaddr *)&dDiscovery.poole_addr, &pAddr); //fd para interaccionar
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
    //el envío de tramas es muy rapido, alomejor no hace falta crear un thread, poco probable que mientras trato una conexion de poole reciba otra de otro poole
    //ademas es mucho gasto recursos
    //cerrar conexion socket!!
    conexionBowman(fd_bowman);
}

void startPooleListener() {

    dDiscovery.fdPoole = socket (AF_INET, SOCK_STREAM, IPPROTO_TCP);    //fd para creacion del socket
    if (dDiscovery.fdPoole < 0) {
        perror ("Error al crear el socket de Poole");
        exit (EXIT_FAILURE);
    } 

    // Specify the adress and port of the socket
    // We'll admit connexions to any IP of our machine in the specified port
    bzero (&dDiscovery.poole_addr, sizeof (dDiscovery.poole_addr));
    dDiscovery.poole_addr.sin_family = AF_INET;
    dDiscovery.poole_addr.sin_port = htons (atoi(dDiscovery.portPoole));
    dDiscovery.poole_addr.sin_addr.s_addr = inet_addr(dDiscovery.ipPoole);

    // When executing bind, we should add a cast:
    // bind waits for a struct sockaddr* and we are passing a struct sockaddr_in*
    if (bind (dDiscovery.fdPoole, (void *) &dDiscovery.poole_addr, sizeof (dDiscovery.poole_addr)) < 0) {
        perror ("Error al enlazar el socket de Poole");
        close(dDiscovery.fdPoole);
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
        sig_func();
    }

    // Specify the adress and port of the socket
    // We'll admit connexions to any IP of our machine in the specified port
    bzero (&dDiscovery.bowman_addr, sizeof (dDiscovery.bowman_addr));
    dDiscovery.bowman_addr.sin_family = AF_INET;
    dDiscovery.bowman_addr.sin_port = htons (atoi(dDiscovery.portBowman));
    dDiscovery.bowman_addr.sin_addr.s_addr = inet_addr(dDiscovery.ipBowman);

    // When executing bind, we should add a cast:
    // bind waits for a struct sockaddr* and we are passing a struct sockaddr_in*
    if (bind (dDiscovery.fdBowman, (void *) &dDiscovery.bowman_addr, sizeof (dDiscovery.bowman_addr)) < 0) {
        perror ("Error al enlazar el socket de Bowman");
        close(dDiscovery.fdBowman);
        sig_func();
    }
    // We now open the port (20 backlog queue, typical value)
    listen (dDiscovery.fdBowman, 20);

    //procesamos las peticiones de Bowman's
    while(1) {
        connect_Bowman();
    }
}

static void *initial_thread_function_bowman() { //revisar si static o no!
    startBowmanListener();
    //pthread_exit(NULL); //revisar! no se puede hacer! hay otra manera! asi se malgasta memoria
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

            //join, buscar la manera de matar el hilo para liberar recuersos.

            //en los casos en los cuales no finalize signals podemos utilizar return null.
        }
    }
    return 0;
}