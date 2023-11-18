/*
Autores:
    Alba Mayol Lozano -->alba.mayol
    Kevin Eljarrat Ohayon --> kevin.eljarrat
*/

#include "Global.h"
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
}

/*
@Finalitat: Manejar la recepción de la signal (SIGINT) y liberar los recursos utilizados hasta el momento.
@Paràmetres: ---
@Retorn: ---
*/
void sig_func() {
    if (dDiscovery.ipPoole != NULL) {
        freeString(dDiscovery.ipPoole);
    }
    if (dDiscovery.portPoole != NULL) {
        freeString(dDiscovery.portPoole);
    }
    if (dDiscovery.ipBowman != NULL) {
        freeString(dDiscovery.ipBowman);
    }
    if (dDiscovery.portBowman != NULL) {
        freeString(dDiscovery.portBowman);
    }
    
    /*if (!LINKEDLIST_isEmpty (dDiscovery.poole_list)) {
        Element client;

        LINKEDLIST_goToHead(&dDiscovery.poole_list);
            
        while(!LINKEDLIST_isAtEnd(dDiscovery.poole_list)) {
            client = LINKEDLIST_get(&dDiscovery.poole_list);
            //kill(client.signalID, SIGINT);
            LINKEDLIST_next(&dDiscovery.poole_list);
        }
        LINKEDLIST_destroy(&dDiscovery.poole_list);
    }*/

    LINKEDLIST_destroy(&dDiscovery.poole_list); //el destroy itera por toda la list haciendo free's de los elementos
    LINKEDLIST_destroy(&dDiscovery.bowman_list);


    exit(EXIT_FAILURE);
}

void conexionPoole(int fd_poole) {
    char *stringTrama = (char *)malloc(sizeof(char)*256);
    read(fd_poole, stringTrama, 256); //read esperando 1a trama
    //convertimos lo leido en trama
    Trama trama = setStringTrama(stringTrama);
    Element element;
    separaDataToElement(trama.data, &element);
    freeTrama(trama);

    printf("nom: %s\n", element.name);
    printf("ip: %s\n", element.ip);
    printf("port: %d\n", element.port);
    printf("num_connections: %d\n", element.num_connections);
    
    //add element as the last one
    LINKEDLIST_goToHead (&dDiscovery.poole_list);
    while(!LINKEDLIST_isAtEnd(dDiscovery.poole_list)) {
        LINKEDLIST_next(&dDiscovery.poole_list);
    }
    LINKEDLIST_add(&dDiscovery.poole_list, element);
    freeElement(&element);


    setTramaString(TramaCreate(0x01, CON_OK, ""), fd_poole);

    
    close(fd_poole);
}



void conexionBowman(int fd_bowman) {
    // Lectura de la trama de Bowman conectado
    char *string = (char *)malloc(sizeof(char) * 256);
    int error = read(fd_bowman, string, 256);
    if (error == -1) {
        perror("Error al recibir la trama");
        close(fd_bowman);
        sig_func();
    }
    
    Trama trama = setStringTrama(string);
    printf("Datos de la trama: %s\n", trama.data);

    //hacemos algo con esta trama?
    freeString(string);
    // Enviar trama con servername, ip y port del Poole
    Element e = pooleMinConnections(&dDiscovery.poole_list);
    //TODO si e.num_connections == -1, no hay elementos en la lista, es decir, no hay Poole's conectados aún
    if (e.num_connections == -1) {
        //NO HAY POOLE'S CONECTADOS! NO PODEMOS REDIRIGIR EL BOWMAN A NINGUN POOLE --> ENVIAMOS TRAMA CON_KO!!!
        setTramaString(TramaCreate(0x01, CON_KO, ""), fd_bowman);
    } else {
        char* aux = NULL;
        char* portString = NULL;
        portString = convertIntToString(e.port);

        aux = createString3Params(e.name, e.ip, portString);
        freeElement(&e);
        freeString(portString);

        setTramaString(TramaCreate(0x01, CON_OK, anadirClaudators(aux)), fd_bowman);
        freeString(aux);
    }

    close(fd_bowman);
}

void connect_Poole() {
    socklen_t pAddr = sizeof(dDiscovery.poole_addr);
    int fd_poole = accept(dDiscovery.fdPoole, (struct sockaddr *)&dDiscovery.poole_addr, &pAddr); //fd para interaccionar
    if (fd_poole < 0) { 
        setTramaString(TramaCreate(0x01, CON_KO, ""), fd_poole);    //TODO REVISAR DONDE VA ESTA TRAMA CON_KO!!!
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