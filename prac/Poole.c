/*
Autores:
    Alba Mayol Lozano -->alba.mayol
    Kevin Eljarrat Ohayon --> kevin.eljarrat
*/

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
        freeString(&dPoole.serverName);
    }
    if (dPoole.pathServerFile != NULL) {
        freeString(&dPoole.pathServerFile);
    }
    if (dPoole.ipDiscovery != NULL) {
        freeString(&dPoole.ipDiscovery);
    }
    if (dPoole.puertoDiscovery != NULL) {
        freeString(&dPoole.puertoDiscovery);
    }
    if (dPoole.ipServer != NULL) {
        freeString(&dPoole.ipServer);
    }
    if (dPoole.puertoServer != NULL) {
        freeString(&dPoole.puertoServer);
    }
    if (dPoole.msg != NULL) {
        freeString(&dPoole.msg);
    }
    exit(EXIT_FAILURE);
}

/*
@Finalitat: Printa la información leída de configPoole
@Paràmetres: ---
@Retorn: ---
*/
void printInfoFile() {
    asprintf(&dPoole.msg, "\nFile read correctly:\nServer - %s\nServer Directory - %s\nIP Discovery - %s\nPort Server - %s\nIP Server - %s\nPort Server - %s\n\n", dPoole.serverName, dPoole.pathServerFile, dPoole.ipDiscovery, dPoole.puertoServer, dPoole.ipServer, dPoole.puertoServer);
    printF(dPoole.msg);
    freeString(&dPoole.msg);
}

void sendSongs(int fd_bowman) {
    printf("%d\n", fd_bowman);
}

void sendPlaylists(int fd_bowman) {
    printf("%d\n", fd_bowman);
}

void requestLogoutBowman(int fd_bowman, int* exit) {
    *exit = 1;
    close(fd_bowman); //close bowman's socket
}

void conexionBowman(int fd_bowman) {
    //TRANSMISIONES POOLE<->BOWMAN
    int exit = 0;

    while(!exit) {
        Trama trama = readTrama(fd_bowman);    
        write(1, trama.data, strlen(trama.data));
        //write(1, trama.header, trama.header_length);
        
   
        if (strcmp(trama.header, "EXIT") == 0) {
            requestLogoutBowman(fd_bowman, &exit);
            printF("Thanks for using HAL 9000, see you soon, music lover!\n");
            break;
        } else if (strcmp(trama.header, "LIST_SONGS") == 0) {
            sendSongs(fd_bowman);
        } else if (strcmp(trama.header, "LIST_PLAYLISTS") == 0) {
            sendPlaylists(fd_bowman);
        } else if (strcmp(trama.header, "CHECK DOWNLOADS") == 0) {
            printF("You have no ongoing or finished downloads\n");
        } else if (strcmp(trama.header, "CLEAR DOWNLOADS") == 0) {
            printF("No downloads to clear available\n");
        } /*else if (strstr(trama.header, "DOWNLOAD") != NULL) {  //DOWNLOAD <SONG/PLAYLIST>
            //comprobar 2 arguments --> 1 espai a la comanda
            int numSpaces = checkDownloadCommand(dBowman.upperInput);
            if (numSpaces == 1) {
                //NUM ARGUMENTS CORRECTE!
                printF("Download started!\n");
            } else {
                printF("Sorry number of arguments is not correct, try again\n");
            }
        } */else {
            printF("Unknown command\n");
        }
        freeTrama(&trama);
    }
}

static void *thread_function_bowman(void* fd) {
    intptr_t fd_bowman_value = (intptr_t)fd;
    int fd_bowman = (int)fd_bowman_value;
    conexionBowman(fd_bowman);

    //pthread_detach(thread_bowman); //revisar! no se puede hacer! hay otra manera! asi se malgasta memoria
    return NULL;
}


void connect_Bowman() {
    socklen_t bAddr = sizeof(dPoole.poole_addr);
    int fd_bowman = accept(dPoole.fdPooleServer, (struct sockaddr *)&dPoole.poole_addr, &bAddr);
    if (fd_bowman < 0) {
        perror("Error al aceptar la conexión de Bowman");
        close(fd_bowman);
        return;
    }

    pthread_t thread_bowman;
    if (pthread_create(&thread_bowman, NULL, thread_function_bowman, (void *)(intptr_t)fd_bowman) != 0) {
        perror("Error al crear el thread inicial para Bowman");
    }
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
    dPoole.poole_addr.sin_addr.s_addr = inet_addr(dPoole.ipServer);

    if (bind(dPoole.fdPooleServer, (struct sockaddr*)&dPoole.poole_addr, sizeof(dPoole.poole_addr)) < 0) {
        perror("Error al enlazar el socket de Poole");
        close(dPoole.fdPooleServer);
        sig_func();
    }

    listen(dPoole.fdPooleServer, 20); // Esperar conexiones entrantes de Bowman
    printF("Waiting for connections...\n");
    //procesamos las peticiones de Bowman's
    while(1) {
        connect_Bowman();
    }    
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
    dPoole.discovery_addr.sin_addr.s_addr = inet_addr(dPoole.ipDiscovery);

    if (connect(dPoole.fdPooleClient, (struct sockaddr*)&dPoole.discovery_addr, sizeof(dPoole.discovery_addr)) < 0) {
        perror("Error al conectar a Discovery");
        close(dPoole.fdPooleClient);
        sig_func();
    }

    //TRANSMISIONES POOLE->DISCOVERY
    
    char* aux = NULL;
    aux = createString3Params(dPoole.serverName, dPoole.ipServer, dPoole.puertoServer);
    setTramaString(TramaCreate(0x01, "NEW_POOLE", aux), dPoole.fdPooleClient);
    freeString(&aux);

    Trama trama = readTrama(dPoole.fdPooleClient);    

    if (strcmp(trama.header,"CON_OK") == 0)  {
        close(dPoole.fdPooleClient);
        freeTrama(&trama);
        waitingForRequests();
    } else if (strcmp(trama.header,"CON_KO") == 0) {
        //PRINT DE QUE NO SE HA PODIDO ESTABLECER LA COMUNICACION CON DISCOVERY
    }
    
    freeTrama(&trama);

    close(dPoole.fdPooleClient);
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
            
            createDirectory(dPoole.serverName); //CREAR DIRECTORIO POOLE
            printInfoFile();
            close(fd);

            establishDiscoveryConnection();

            sig_func();
        }
    }

    return 0;
}