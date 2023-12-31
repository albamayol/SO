/*
Autores:
    Alba Mayol Lozano -->alba.mayol
    Kevin Eljarrat Ohayon --> kevin.eljarrat
*/

#include "Trama.h"

dataBowman dBowman;
    
typedef struct {
    char type;
    short header_length; 
    char header[256];
    char data[256];
} TramaAux;

typedef struct {
    long idmsg;
    TramaAux trama;
} MissatgeAux;

typedef struct {
    long mtype;
    char type;
    short header_length;
    char mtext[256];
    //char data[256];
} MissatgeAuxAux;

typedef struct {
    long idmsg;
    Trama trama;
} Missatge;

Missatge msgGap;

int requestLogout();
/*
@Finalitat: Inicializar las variables a NULL.
@Paràmetres: ---
@Retorn: ---
*/
void inicializarDataBowman() {
    dBowman.msg = NULL;
    dBowman.input = NULL;
    dBowman.upperInput = NULL;
    dBowman.clienteNameAux = NULL;
    dBowman.clienteName = NULL;
    dBowman.pathClienteFile = NULL;
    dBowman.ip = NULL;
    dBowman.puerto = NULL;
    dBowman.bowmanConnected = 0;
    dBowman.infoPlaylists = NULL;
}
/*
@Finalitat: Manejar la recepción de la signal (SIGINT) y liberar los recursos utilizados hasta el momento.
@Paràmetres: ---
@Retorn: ---
*/
void sig_func() {
    // limpiar los threads descargas. TODO

    if(dBowman.bowmanConnected) {   //Si bowman està connectat a poole
        if (requestLogout()) {
            printF("Thanks for using HAL 9000, see you soon, music lover!\n");
        } 
    }
    if(dBowman.upperInput != NULL) {
        freeString(&dBowman.upperInput);
    }
    if(dBowman.msg != NULL) {
        freeString(&dBowman.msg);
    }
    if(dBowman.input != NULL) {
        freeString(&dBowman.input);
    }
    if(dBowman.clienteName != NULL) {
        freeString(&dBowman.clienteName);
    }
    if(dBowman.clienteNameAux != NULL) {
        freeString(&dBowman.clienteNameAux);
    }
    if(dBowman.pathClienteFile != NULL) {
        freeString(&dBowman.pathClienteFile);
    }
    if(dBowman.ip != NULL) {
        freeString(&dBowman.ip);
    }
    if(dBowman.puerto != NULL) {
        freeString(&dBowman.puerto);
    }
    freeElement(&dBowman.pooleConnected);

    // Eliminar cola de mensajes peticiones
    /*if (msgctl(dBowman.msgQueuePetitions, IPC_RMID, NULL) == -1) {
        perror("Error al eliminar la cola de mensajes");
    }*/
    msgctl (dBowman.msgQueuePetitions, IPC_RMID, (struct msqid_ds *)NULL);

    // Eliminar cola de mensajes descargas canciones
    /*if (msgctl(dBowman.msgQueueDescargas, IPC_RMID, NULL) == -1) {
        perror("Error al eliminar la cola de mensajes");
    }*/

    msgctl (dBowman.msgQueueDescargas, IPC_RMID, (struct msqid_ds *)NULL);

    // Liberamos los recursos del thread de lectura
    pthread_cancel(dBowman.threadRead);
    pthread_join(dBowman.threadRead, NULL);

    cleanAllTheThreadsBowman(&dBowman.descargas, dBowman.numDescargas);

    exit(EXIT_FAILURE);
}

/*
@Finalitat: Printa la información leída de configBowman
@Paràmetres: ---
@Retorn: ---
*/
void printInfoFileBowman() {
    printF("\nFile read correctly:\n");
    asprintf(&dBowman.msg, "User - %s\nDirectory - %s\nIP - %s\nPort - %s\n\n", dBowman.clienteName, dBowman.pathClienteFile, dBowman.ip, dBowman.puerto);
    printF(dBowman.msg);
    freeString(&dBowman.msg);
}

char* read_until_string(char *string, char delimiter) {
    int i = 0;
    char *msg = NULL;

    while (string[i] != delimiter && string[i] != '\0') {
        msg = realloc(msg, i + 2); // Incrementamos el tamaño para el carácter extra y el terminador
        if (msg == NULL) {
            perror("Error en realloc");
            exit(EXIT_FAILURE);
        }
        msg[i] = string[i];
        i++;
    }

    msg[i] = '\0';

    return msg;
}

void establishDiscoveryConnection() {
    dBowman.fdDiscovery = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (dBowman.fdDiscovery < 0) {
        perror ("Error al crear el socket de Discovery");
        close(dBowman.fdDiscovery);
        sig_func();
    }

    bzero (&dBowman.discovery_addr, sizeof (dBowman.discovery_addr));
    dBowman.discovery_addr.sin_family = AF_INET;
    dBowman.discovery_addr.sin_port = htons(atoi(dBowman.puerto)); 
    dBowman.discovery_addr.sin_addr.s_addr = inet_addr(dBowman.ip);

    if (connect(dBowman.fdDiscovery, (struct sockaddr*)&dBowman.discovery_addr, sizeof(dBowman.discovery_addr)) < 0) {
        perror("Error al conectar a Discovery");
        close(dBowman.fdDiscovery);
        sig_func();
    }

    //TRANSMISIONES DISCOVERY->BOWMAN
    char *aux = NULL;

    int length = strlen(dBowman.clienteName) + 3;
    aux = (char *) malloc(sizeof(char) * length);
    for (int i = 0; i < length; i++) {
        aux[i] = '\0';
    }

    strcpy(aux, dBowman.clienteName);
    setTramaString(TramaCreate(0x01, "NEW_BOWMAN", aux, strlen(aux)), dBowman.fdDiscovery);
    freeString(&aux);

    TramaExtended tramaExtended = readTrama(dBowman.fdDiscovery);

    if (strcmp(tramaExtended.trama.header,"CON_OK") == 0)  {
        separaDataToElement(tramaExtended.trama.data, &dBowman.pooleConnected);
        asprintf(&dBowman.msg, "%s connected to HAL 9000 system, welcome music lover!\n", dBowman.clienteName);
        printF(dBowman.msg);
        freeString(&dBowman.msg);
    } else if (strcmp(tramaExtended.trama.header,"CON_KO") == 0) {
        write(1, "CON_KO\n", strlen("CON_KO\n"));
    }

    freeTrama(&(tramaExtended.trama));
    close(dBowman.fdDiscovery);
}

void checkPooleConnection() {
    dBowman.bowmanConnected = 0;
    close(dBowman.fdPoole);
    asprintf(&dBowman.msg, "\n¡Alert: %s disconnected because the server connection has ended!\n", dBowman.clienteName);
    printF(dBowman.msg);
    freeString(&dBowman.msg);
    sig_func();
}

static void *thread_function_read() {
    while(1) {
        //que finalize cuando se desconecte el cliente
        MissatgeAuxAux msg;
        memset(&msg, 0, sizeof(MissatgeAuxAux));

        TramaExtended tramaExtended = readTrama(dBowman.fdPoole); 

        // Comprobación si Poole ha cerrado conexión
        if (tramaExtended.initialized) {
            checkPooleConnection();
        }
        
        msg.mtype = tramaExtended.trama.type;
        msg.header_length = tramaExtended.trama.header_length;
        //msg.trama.header = strdup(tramaExtended.trama.header);
        //msg.trama.data = strdup(tramaExtended.trama.data);

        size_t i;
        memset(msg.mtext, 0, 256);
        //msg.header = {0};
        for (i = 0; i < strlen(tramaExtended.trama.header); i++) {
            msg.mtext[i] = tramaExtended.trama.header[i];
        }
        msg.mtext[i] = '\0';

        /*memset(msg.data, 0, 256);
        //msg.data = {0};
        for (i = 0; i < strlen(tramaExtended.trama.data); i++) {
            msg.data[i] = tramaExtended.trama.data[i];
        }
        msg.data[i] = '\0';*/

        size_t totalSize = sizeof(MissatgeAuxAux) - sizeof(long);
        //CRIBAJE --> añadir mensaje a msg queue de peticiones o msg queue de descargas
        //CUANDO LEAMOS UN MESSAGE DE TIPO X, COMO NOS QUEDARÀ EL GAP Y SE AÑADIRIA LA PROXIMA TRAMA, JUSTO AL HACER RCV HACEMOS EN LA LINIA DE ABAJO RELLENAMOS EL GAP DEJADO CON UN MENSAJE CON TIPO 5000, QUE NUNCA SE VA A DAR
        if (strcmp(tramaExtended.trama.header, "FILE_DATA") == 0) { 
            char* stringID = read_until_string(tramaExtended.trama.data, '&'); //cribaje segun idsong!!! UNA SOLA QUEUE MSG QUE EL TYPE DEL MSG SERA EL ID DE LA SONG
            msg.mtype = atoi(stringID);
            freeString(&stringID);

            if (msgsnd(dBowman.msgQueueDescargas, &msg, totalSize, IPC_NOWAIT) == -1) { //IPC_NOWAIT HACE QUE SI LA QUEUE SE LLENA, NO SALTE CORE DUMPED, SINO QUE SE BLOQUEE LA QUEUE (EFECTO BLOQUEANTE)
                perror("msgsnd"); 
                //exit(EXIT_FAILURE);
            }
        } else {
            if (strcmp(tramaExtended.trama.header, "CONOK") == 0 || strcmp(tramaExtended.trama.header, "CONKO") == 0) {                                    //LOGOUT
                //msg.idmsg = 3;
                msg.mtype = 3;
            } else if (strcmp(tramaExtended.trama.header, "SONGS_RESPONSE") == 0) {                                                           //LIST SONGS
                msg.mtype = 1;
            } else if (strcmp(tramaExtended.trama.header, "PLAYLISTS_RESPONSE") == 0) {                                                       //LIST PLAYLISTS
                msg.mtype = 2;
            } else if (strcmp(tramaExtended.trama.header, "CON_OK") == 0 || strcmp(tramaExtended.trama.header, "CON_KO") == 0) {                            //CONNECT POOLE
                msg.mtype = 0;
            } else if (strcmp(tramaExtended.trama.header, "PLAY_EXIST") == 0 || strcmp(tramaExtended.trama.header, "PLAY_NOEXIST") == 0) {                  //CHECK PLAYLIST EXIST 
                msg.mtype = 5;
            } else if (strcmp(tramaExtended.trama.header, "FILE_NOEXIST") == 0 || strcmp(tramaExtended.trama.header, "FILE_EXIST") == 0) {                  //CHECK SONG EXIST
                msg.mtype = 4;
            } else if (strcmp(tramaExtended.trama.header, "NEW_FILE") == 0) {                                                                 //NEW_FILE 
                msg.mtype = 6;
            }
            asprintf(&dBowman.msg, "\nId msgQueue %d, Id mensaje: %ld, type: %d, length %hd, header: %s", dBowman.msgQueuePetitions, msg.mtype, msg.type, msg.header_length, msg.mtext);
            printF(dBowman.msg);
            freeString(&dBowman.msg);

            if (msgsnd(dBowman.msgQueuePetitions, &msg, 257*sizeof(char) + sizeof(short), IPC_NOWAIT) == -1) { //IPC_NOWAIT HACE QUE SI LA QUEUE SE LLENA, NO SALTE CORE DUMPED, SINO QUE SE BLOQUEE LA QUEUE (EFECTO BLOQUEANTE)
                perror("msgsnd"); 
                //exit(EXIT_FAILURE);
            }
        } 
        freeTrama(&(tramaExtended.trama));
        //freeTrama(&(msg.trama));
    }
    return NULL; 
}

void creacionHiloLectura() {
    if (pthread_create(&dBowman.threadRead, NULL, thread_function_read, NULL) != 0) {
        perror("Error al crear el thread de lectura\n");
    }
}

void establishPooleConnection() {
    dBowman.fdPoole = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (dBowman.fdPoole < 0) {
        perror ("Error al crear el socket de Poole");
        close(dBowman.fdPoole);
        sig_func();
    }

    bzero (&dBowman.poole_addr, sizeof (dBowman.poole_addr));
    dBowman.poole_addr.sin_family = AF_INET;
    dBowman.poole_addr.sin_port = htons(dBowman.pooleConnected.port); 
    dBowman.poole_addr.sin_addr.s_addr = inet_addr(dBowman.pooleConnected.ip);

    if (connect(dBowman.fdPoole, (struct sockaddr*)&dBowman.poole_addr, sizeof(dBowman.poole_addr)) < 0) {
        perror("Error al conectar a Poole");
        close(dBowman.fdPoole);
        sig_func();
    }

    creacionHiloLectura();
    
    // Transmission Bowman->Poole
    setTramaString(TramaCreate(0x01, "NEW_BOWMAN", dBowman.clienteName, strlen(dBowman.clienteName)), dBowman.fdPoole);
    

    // Recepción Poole->Bowman para comprobar el estado de la conexion.
    Missatge msg;
    //size_t message_size = sizeof(long) + sizeof(short) + strlen(msg.trama.header) + strlen(msg.trama.data) + 2;
    if (msgrcv(dBowman.msgQueuePetitions, &msg, sizeof(Missatge) - sizeof(long), 0, 0) == -1) {
        printF("ERROR");
    }    
    
     // Aqui no hace falta llenar el gap porque solo esperamos 1 trama.
    //Trama trama = readTrama(dBowman.fdPoole);
    if (strcmp(msg.trama.header, "CON_OK") == 0) {
        dBowman.bowmanConnected = 1;
        printF("KEV");
    } else if (strcmp(msg.trama.header, "CON_KO") == 0) {
        close(dBowman.fdPoole);
    }

    freeTrama(&(msg.trama));
}

void juntarTramasSongs(int numTramas, char **songs) {
    int i = 0;
    //char aux[257];
    size_t totalSize = 0; 
    Missatge msg;

    while(i < numTramas) {
        //read(dBowman.fdPoole, aux, 256);
        //aux[256] = '\0';

        msgrcv(dBowman.msgQueuePetitions, &msg, sizeof(Missatge), 1, 0);
        
        msgsnd(dBowman.msgQueuePetitions, &msgGap, sizeof(Missatge), IPC_NOWAIT);

        //size_t dataSize = strlen(aux + 17);
        cleanPadding(msg.trama.data, '~');
        size_t dataSize = strlen(msg.trama.data);
        *songs = realloc(*songs, totalSize + dataSize + 1);
        if (*songs == NULL) {
            break;
        }

        // Copiamos los datos de la trama actual a songs
        //memcpy(*songs + totalSize, aux + 17, dataSize);
        memcpy(*songs + totalSize, msg.trama.data, dataSize);
        totalSize += dataSize;

        (*songs)[totalSize] = '\0';
        i++;
        freeTrama(&(msg.trama));
    }
}

int procesarTramasSongs(char ***canciones, char *songs) {
    int numCanciones = 0, inicio = 0;

    char final = ' ', *song = NULL;

    do {
        song = readUntilFromIndex(songs, &inicio, '&', &final, '~');

        *canciones = realloc(*canciones, (numCanciones + 1) * sizeof(char *));
        if (*canciones == NULL) {
            break;
        }
        (*canciones)[numCanciones] = song;
        numCanciones++;
    } while (final != '~');
    return numCanciones;
}

void printarSongs(int numCanciones, char ***canciones) {
    asprintf(&dBowman.msg, "\nThere are %d songs available for download:", numCanciones);
    printF(dBowman.msg);
    freeString(&dBowman.msg);

    for (int i = 0; i < numCanciones; i++) {
        if (i == numCanciones - 1) {
            asprintf(&dBowman.msg, "\n%d. %s\n\n", i + 1, (*canciones)[i]);
        } else {
            asprintf(&dBowman.msg, "\n%d. %s", i + 1, (*canciones)[i]);
        }

        printF(dBowman.msg);

        freeString(&dBowman.msg);
        free((*canciones)[i]);
    }
}

void requestListSongs() {
    int numCanciones = 0;
    char *songs = NULL, **canciones = NULL;
    //char aux[257];

    setTramaString(TramaCreate(0x02, "LIST_SONGS", "", 0), dBowman.fdPoole);

    // Lectura cantidad de tramas que recibiremos
    //ssize_t bytesLeidos = read(dBowman.fdPoole, aux, 256);
    //aux[256] = '\0';

    Missatge msg;
    msgrcv(dBowman.msgQueuePetitions, &msg, sizeof(Missatge), 1, 0);

    msgsnd(dBowman.msgQueuePetitions, &msgGap, sizeof(Missatge), IPC_NOWAIT);

    //checkPooleConnection(bytesLeidos);

    //int numTramas = atoi(aux + 17);
    cleanPadding(msg.trama.data, '~');
    int numTramas = atoi(msg.trama.data);

    juntarTramasSongs(numTramas, &songs);
    numCanciones = procesarTramasSongs(&canciones, songs);
    printarSongs(numCanciones, &canciones);

    free(canciones);
    freeString(&songs);
    freeTrama(&(msg.trama));
}

char *juntarTramasPlaylists(int numTramas) {
    int i = 0;
    char *playlists = NULL;
    //char aux[257];
    ssize_t totalSize = 0; 
    Missatge msg;

    while(i < numTramas) {

        msgrcv(dBowman.msgQueuePetitions, &msg, sizeof(Missatge), 2, 0);

        msgsnd(dBowman.msgQueuePetitions, &msgGap, sizeof(Missatge), IPC_NOWAIT);

        //read(dBowman.fdPoole, aux, 256);
        //aux[256] = '\0';

        //int dataSize = strlen(aux + 17);
        cleanPadding(msg.trama.data, '~');
        int dataSize = strlen(msg.trama.data);

        playlists = realloc(playlists, totalSize + dataSize + 1);
        if (playlists == NULL) {
            break;
        }

        // Copiamos los datos de la trama actual a songs
        //memcpy(playlists + totalSize, aux + 17, dataSize);
        memcpy(playlists + totalSize, msg.trama.data, dataSize);
        totalSize += dataSize;

        playlists[totalSize] = '\0';
        i++;
        freeTrama(&(msg.trama));
    }

    size_t len = strlen(playlists);
    playlists = realloc(playlists, len + 2);
    if (playlists != NULL) {
        playlists[len] = '#';
        playlists[len + 1] = '\0';
    }
    return playlists;
}

char ***procesarTramasPlaylists(char *playlists, int **numCancionesPorLista, int numCanciones, int *numListas) {
    int i = 0, totalCanciones = 0, inicioPlaylist = 0, inicioSong = 0;
    char valorFinal = ' ', ***listas = NULL, *playlist = NULL, *song = NULL;

    do {
        playlist = readUntilFromIndex(playlists, &inicioPlaylist, '#', &valorFinal, '~');

        size_t len = strlen(playlist);
        playlist = realloc(playlist, len + 2);
        if (playlist == NULL) {
            break;
        }
        playlist[len] = '#';
        playlist[len + 1] = '\0';

        inicioSong = 0;
        i = 0;
        valorFinal = ' ';
        do {    
            song = readUntilFromIndex(playlist, &inicioSong, '&', &valorFinal, '#');
            if (i == 0) {
                // Primero, reservamos memoria para almacenar una nueva lista
                listas = realloc(listas, ((*numListas) + 1) * sizeof(char **));
                if (listas == NULL) {
                    break;
                }
                // Luego, reservamos memoria para el nombre de la lista (el primer elemento de la lista)
                listas[*numListas] = malloc(sizeof(char *));
                if (listas[*numListas] == NULL) {
                    break;
                }
                printF(song);
                // Guardamos el nombre de la lista
                listas[*numListas][i] = strdup(song); 
                if (listas[*numListas][i] == NULL) {
                    free(listas[*numListas]);
                    break;
                }
            } else {
                // Realocamos memoria para un nuevo elemento en la lista, en este caso para la nueva canción
                listas[*numListas] = realloc(listas[*numListas], (i + 1) * sizeof(char *));
                if (listas[*numListas] == NULL) {
                    break;
                }

                // Guardamos la nueva canción
                listas[*numListas][i] = strdup(song);
                if (listas[*numListas][i] == NULL) {
                    for (int k = 0; k < i; k++) {
                        free(listas[*numListas][k]);
                    }
                    free(listas[*numListas]);
                    break;
                }

                (*numCancionesPorLista)[*numListas]++; 
            }
            i++;
            freeString(&song);
        } while (valorFinal != '#');
        totalCanciones += (*numCancionesPorLista)[*numListas];
        (*numListas)++;

        (*numCancionesPorLista) = realloc((*numCancionesPorLista), (*numListas + 1) * sizeof(int));
        (*numCancionesPorLista)[(*numListas)] = 0;
        if (*numCancionesPorLista == NULL) {
            break;
        }
        freeString(&playlist);
    } while (totalCanciones < numCanciones);

    return listas;
}

int numSongsDePlaylist(InfoPlaylist* infoPlaylists, char* listName) {
    for (int i = 0; i < dBowman.numInfoPlaylists; i++) {
        if (strcmp(infoPlaylists[i].nameplaylist, listName) == 0) {
            return infoPlaylists[i].numSongs; //devolvemos numero de canciones que tiene esa playlist
        }
    }
    return -1;
}

void printarPlaylists(int numListas, char ***listas, int *numCancionesPorLista) {
    asprintf(&dBowman.msg, "\nThere are %d lists available for download:", numListas);
    printF(dBowman.msg);
    freeString(&dBowman.msg);

    for (int i = 0; i < numListas; i++) {
        asprintf(&dBowman.msg, "\n%d. %s", i + 1, listas[i][0]);
        printF(dBowman.msg);
        freeString(&dBowman.msg);

        //guardamos info Playlists actuales
        dBowman.infoPlaylists = realloc(dBowman.infoPlaylists, sizeof(InfoPlaylist) * dBowman.numInfoPlaylists + 1);
        dBowman.infoPlaylists[dBowman.numInfoPlaylists].nameplaylist = strdup(listas[i][0]); 
        dBowman.infoPlaylists[dBowman.numInfoPlaylists].numSongs = numCancionesPorLista[i];
        dBowman.numInfoPlaylists++;

        free(listas[i][0]);
        
        for (int j = 1; j <= numCancionesPorLista[i]; j++) {
            asprintf(&dBowman.msg, "\n   %c. %s", 'a' + j - 1, listas[i][j]);
            printF(dBowman.msg);
            freeString(&dBowman.msg);
            free(listas[i][j]);
        }

        free(listas[i]); 
    }
    printF("\n\n");

    free(listas); 
    free(numCancionesPorLista);
}

void requestListPlaylists() {
    char *playlists = NULL, ***listas = NULL;
    //char aux[257], 
    int numListas = 0;
    int *numCancionesPorLista = malloc(sizeof(int)); 
    *numCancionesPorLista = 0;
    Missatge msg;

    setTramaString(TramaCreate(0x02, "LIST_PLAYLISTS", "", 0), dBowman.fdPoole);

    // Lectura cantidad de canciones
    //ssize_t bytesLeidos = read(dBowman.fdPoole, aux, 256);
    //aux[256] = '\0';

    msgrcv(dBowman.msgQueuePetitions, &msg, sizeof(Missatge), 2, 0);
    msgsnd(dBowman.msgQueuePetitions, &msgGap, sizeof(Missatge), IPC_NOWAIT);

    freeTrama(&(msg.trama));

    //checkPooleConnection(bytesLeidos);

    //int numCanciones = atoi(aux + 17);
    cleanPadding(msg.trama.data, '~');
    int numCanciones = atoi(msg.trama.data);

    // Lectura cantidad de tramas que recibiremos
    //read(dBowman.fdPoole, aux, 256);
    //aux[256] = '\0';
    msgrcv(dBowman.msgQueuePetitions, &msg, sizeof(Missatge), 2, 0);
    msgsnd(dBowman.msgQueuePetitions, &msgGap, sizeof(Missatge), IPC_NOWAIT);

    //int numTramas = atoi(aux + 17);
    int numTramas = atoi(msg.trama.data);

    playlists = juntarTramasPlaylists(numTramas);

    listas = procesarTramasPlaylists(playlists, &numCancionesPorLista, numCanciones, &numListas);
    
    printarPlaylists(numListas, listas, numCancionesPorLista);

    free(playlists);
}

int requestLogout() {  
    setTramaString(TramaCreate(0x06, "EXIT", dBowman.clienteName, strlen(dBowman.clienteName)), dBowman.fdPoole);

    Missatge msg;
    //char aux[257];
    //ssize_t bytesLeidos = read(dBowman.fdPoole, aux, 256);  
    //aux[256] = '\0';

    msgrcv(dBowman.msgQueuePetitions, &msg, sizeof(Missatge), 3, 0); //Aqui no hace falta llenar el gap porque solo recibiras una trama

    //dBowman.bowmanConnected = 0;
    /*if (bytesLeidos <= 0) {
        close(dBowman.fdPoole);
        asprintf(&dBowman.msg, "\n¡Alert: %s disconnected because the server connection has ended!\n", dBowman.clienteName);
        printF(dBowman.msg);
        freeString(&dBowman.msg);
        return 1;
    }*/

    /*char header[6]; //CONOK o CONKO
    int indiceInicio = 3;
    int longitudHeader = 8 - 3; 

    // Usamos strncpy para copiar la subcadena desde aux a subcadena
    strncpy(header, aux + indiceInicio, longitudHeader);
    header[longitudHeader] = '\0';*/

    if (strcmp(msg.trama.header, "CONOK") == 0) {
        printF(msg.trama.header);
        //OK
        close(dBowman.fdPoole);
        freeTrama(&(msg.trama));
        return 1;
    } else if (strcmp(msg.trama.header, "CONKO")) {
        printF(msg.trama.header);
        //KO
        printF("Sorry, couldn't logout, try again\n");
        freeTrama(&(msg.trama));
        return 0;
    }
    freeTrama(&(msg.trama));
    return 2;
}

void getIdData(char* buffer, char** dataFile, DescargaBowman *mythread) {    //separamos id & datos archivo
    int counter = 0, i = 0; //lengthData = 244; //256 - (1 - 2 - 9 (header: "FILE_DATA")) = 244

    //primero obtenemos el id
    while (buffer[counter] != '&') {
        counter++;
    }
    counter++; //saltamos &

    while ((counter < 244) && mythread->song.bytesDescargados < mythread->song.size) {    
        *dataFile = (char*) realloc (*dataFile, i + 1);
        (*dataFile)[i] = buffer[counter];
        i++;
        counter++;

        mythread->song.bytesDescargados++; 
    }
}

int min(size_t a, size_t b) {
    if (a < b) {
        return a;
    }
    return b;
}

void createMP3FileInDirectory(char* directory, DescargaBowman *mythread, size_t size, int idSong) {
    //int id = 0; 
    char* dataFile = NULL;
    //char buffer[256];
    //ssize_t bytesLeidos = 0;
    Missatge msg;

    size_t len = strlen(directory) + strlen(mythread->song.nombre) + 2;
    char *path = malloc(len);
    snprintf(path, len, "%s/%s", directory, mythread->song.nombre);//Floyd/sutton/song2

    // Creamos el archivo .mp3
    int fd_file = open(path, O_CREAT | O_RDWR, 0644); 
    if (fd_file == -1) {
        perror("Error al crear el archivo");
        freeString(&path);
        freeString(&dataFile);
        sig_func();
    }
    size_t file_size = size;
    int sizeDataTrama = 256 - 12 - strlen(convertIntToString(idSong)) - 1;
    do {
        //bytesLeidos = read(dBowman.fdPoole, buffer, 256); //FILE_DATA --> dynamic msg queue
        msgrcv(dBowman.msgQueueDescargas, &msg, sizeof(Missatge), idSong, 0);
        msgsnd(dBowman.msgQueueDescargas, &msgGap, sizeof(Missatge), IPC_NOWAIT);
        //CONTINUAR AQUI
        /*if (bytesLeidos == -1) {
            perror("Error al leer desde el file descriptor de Poole");
            break;
        }*/
        //getIdData(buffer + 12, &dataFile, mythread);
        getIdData(msg.trama.data, &dataFile, mythread);

        if (write(fd_file, dataFile, min(file_size, sizeDataTrama)) == -1) { // Escribir lo leído en el archivo
            perror("Error al escribir en el archivo");
            break;
        }
        file_size -= sizeDataTrama;
        //Actualizamos porcentaje.
        mythread->porcentaje = ((float)mythread->song.bytesDescargados / mythread->song.size) * 100;
        freeString(&dataFile);
        freeTrama(&(msg.trama));
    } while (file_size > 0); 

    //COMPROBACIÓN MD5SUM
    char *md5sum = resultMd5sumComand(path);
    printF("mdsum descarga: ");
    printF(md5sum);
    printF("\n");
    if (md5sum != NULL) {
        if (strcmp(md5sum, mythread->song.md5sum) == 0) {
            //OK
            setTramaString(TramaCreate(0x05, "CHECK_OK", "", 0), dBowman.fdPoole);
        } else {
            //KO
            setTramaString(TramaCreate(0x05, "CHECK_KO", "", 0), dBowman.fdPoole);
        }
        freeString(&md5sum);
    }
    
    close(fd_file);
    freeString(&path);
}

void downloadSong(DescargaBowman *mythread) {
    char valorFinal = ' ';
    //char aux[257], 
    int inicio = 0, i = 1;

    printF("Download started!\n");
    //leemos cola mensajes estatica
    //ssize_t bytesLeidos = read(dBowman.fdPoole, aux, 256); // NEW_FILE 
    //aux[256] = '\0';
    Missatge msg;

    msgrcv(dBowman.msgQueuePetitions, &msg, sizeof(Missatge), 6, 0);

    //checkPooleConnection(bytesLeidos);

    //char *dataSong = read_until_string(&aux[11], '~');
    cleanPadding(msg.trama.data, '~');
    char *dataSong = msg.trama.data; 

    while (valorFinal != '\0') {
        char *paramDataSong = readUntilFromIndex(dataSong, &inicio, '&', &valorFinal, '\0');
        switch (i) {
            case 1: mythread->song.nombre = strdup(paramDataSong);
                break;
            case 2: mythread->song.size = atoi(paramDataSong);
                break;
            case 3: mythread->song.md5sum = strdup(paramDataSong); //Floyd/sutton
                break;
            case 4: mythread->song.id = atoi(paramDataSong);
                break;
        }
        freeString(&paramDataSong);
        i++;
    }    
    freeString(&dataSong);
    freeTrama(&(msg.trama));
    createMP3FileInDirectory(dBowman.clienteName, mythread, mythread->song.size, mythread->song.id);
}

static void *thread_function_download_song(void* thread) {
    DescargaBowman *mythread = (DescargaBowman*) thread;

    downloadSong(mythread);
    return NULL; 
}

void threadDownloadSong(char *song) {   
    dBowman.descargas = realloc(dBowman.descargas, sizeof(DescargaBowman) * (dBowman.numDescargas + 1)); 
    dBowman.descargas[dBowman.numDescargas].nombreDescargaComando = strdup(song);
    dBowman.descargas[dBowman.numDescargas].porcentaje = 0.00; 
    dBowman.descargas[dBowman.numDescargas].song.bytesDescargados = 0; 
    //dBowman.descargas[dBowman.numDescargas].song.pathSong = NULL;
    dBowman.numDescargas++;

    if (pthread_create(&dBowman.descargas[dBowman.numDescargas - 1].thread, NULL, thread_function_download_song, (void *)&dBowman.descargas[dBowman.numDescargas - 1]) != 0) {
        perror("Error al crear el thread para la descarga");
        dBowman.numDescargas--;
    }

    freeString(&song);
}

void requestDownloadSong(char* nombreArchivoCopia) {
    setTramaString(TramaCreate(0x03, "DOWNLOAD_SONG", nombreArchivoCopia, strlen(nombreArchivoCopia)), dBowman.fdPoole); //playlistname / songname
    //ESPERAMOS TRAMA SI SONG EXISTE O NO
    //Trama trama = readTrama(dBowman.fdPoole);
    Missatge msg;
    msgrcv(dBowman.msgQueuePetitions, &msg, sizeof(Missatge), 4, 0);

    if (strcmp(msg.trama.header, "FILE_EXIST") == 0) {
        freeTrama(&(msg.trama));
        threadDownloadSong(nombreArchivoCopia);
    } else if (strcmp(msg.trama.header, "FILE_NOEXIST") == 0) {
        freeTrama(&(msg.trama));
    }    
}

void requestDownloadPlaylist(char* nombreArchivoCopia) {
    setTramaString(TramaCreate(0x03, "DOWNLOAD_LIST", nombreArchivoCopia, strlen(nombreArchivoCopia)), dBowman.fdPoole); //playlistname / songname

    //ESPERAMOS TRAMA SI PLAYLIST EXISTE O NO
    //Trama trama = readTrama(dBowman.fdPoole);

    Missatge msg;
    msgrcv(dBowman.msgQueuePetitions, &msg, sizeof(Missatge), 5, 0);

    if (strcmp(msg.trama.header, "PLAY_EXIST") == 0) {
        freeTrama(&(msg.trama));

        char *playlistDirectory = malloc(strlen(dBowman.clienteName) + strlen(nombreArchivoCopia) + 2); // +2 para / y \0
        sprintf(playlistDirectory, "%s/%s", dBowman.clienteName, nombreArchivoCopia);   
        createDirectory(playlistDirectory);
        freeString(&playlistDirectory);

        //por cada cancion de la playlist creamos su thread
        int numSongs = numSongsDePlaylist(dBowman.infoPlaylists, nombreArchivoCopia);
        for (int i = 0; i < numSongs; i++) {
            threadDownloadSong(nombreArchivoCopia);
        }
    } else if (strcmp(msg.trama.header, "PLAY_NOEXIST") == 0) {
        freeTrama(&(msg.trama));
    }
}

void creacionMsgQueues() {
    //[0] --> connect Poole
    //[1] --> list songs
    //[2] --> lists playlists
    //[3] --> logout
    //[4] --> check download song
    //[5] --> check download playlist
    //[6] --> NEW_FILE

    key_t key1 = ftok("Bowman.c", 0xCA);

    int id_queue = msgget(key1, 0600 | IPC_CREAT);
    if (id_queue < 0) {
        write(1, "Error al crear la cua de missatges de les peticions\n", strlen("Error al crear la cua de missatges de les peticions\n"));
        return;
    }
    dBowman.msgQueuePetitions = id_queue;
    //printf("Id msgQueuePetitions %d\n", id_queue);

    key_t key2 = ftok("Bowman.c", 0xCB);

    //creamos msgqueue para descargas de songs
    id_queue = msgget(key2, 0600 | IPC_CREAT);
    if (id_queue < 0) {
        write(1, "Error al crear la cua de missatges de les descargues\n", strlen("Error al crear la cua de missatges de les descargues\n"));
        return;
    }
    dBowman.msgQueueDescargas = id_queue;
    //printf("Id msgQueueDescargas %d\n", id_queue);
}

void showDownloadStatus(DescargaBowman *descargas, int numDescargas) {
    for (int i = 0; i < numDescargas; i++) {
        if (descargas[i].song.nombre != NULL) {
            // Descarga no eliminada
            if (strcmp(descargas[i].song.nombre, descargas[i].nombreDescargaComando) == 0) {
                //Es una cancion
                printF(descargas[i].song.nombre);
                printF("\n");
            } else {
                // Es una cancion de una playlist
                asprintf(&dBowman.msg, "%s - %s\n", descargas[i].nombreDescargaComando, descargas[i].song.nombre);
                printF(dBowman.msg);
                freeString(&dBowman.msg);
            }
            int numeroEqualChar = (int)(descargas[i].porcentaje / 5.0); // Asumimos que un 100% equivale a 20('=').
                
            char *cadena = malloc(numeroEqualChar + 1);
            int j = 0;
            for (j = 0; j < numeroEqualChar; j++) {
                cadena[j] = '=';
            }
            cadena[j] = '\0';

            asprintf(&dBowman.msg, "\t%.2f%% |%s%%|\n", descargas[i].porcentaje, cadena);
            printF(dBowman.msg);
            freeString(&dBowman.msg);
            freeString(&cadena);
        }
    }
}

/*
@Finalitat: Implementar el main del programa.
@Paràmetres: ---
@Retorn: int: Devuelve 0 en caso de que el programa haya finalizado exitosamente.
*/
int main(int argc, char ** argv) {
    inicializarDataBowman();

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
            dBowman.clienteNameAux = read_until(fd, '\n');

            dBowman.clienteName = verifyClientName(dBowman.clienteNameAux);
            freeString(&dBowman.clienteNameAux);

            dBowman.pathClienteFile = read_until(fd, '\n');
            dBowman.ip = read_until(fd, '\n');
            dBowman.puerto = read_until(fd, '\n');

            asprintf(&dBowman.msg, "\n%s user initialized\n", dBowman.clienteName);
            printF(dBowman.msg);
            freeString(&dBowman.msg);

            createDirectory(dBowman.clienteName);

            printInfoFileBowman();

            creacionMsgQueues();

            msgGap.idmsg = 1000;
            
            while (1) { 
                printF("$ ");
                dBowman.input = read_until(0, '\n'); 
                dBowman.input[strlen(dBowman.input)] = '\0';
                
                dBowman.upperInput = to_upper(dBowman.input);
                removeExtraSpaces(dBowman.upperInput);

                if (!dBowman.bowmanConnected) {
                    if (strcmp(dBowman.upperInput, "CONNECT") == 0) {
                        establishDiscoveryConnection();
                        establishPooleConnection();
                    } else {
                        printF("You must establish a connection with the server before making any request\n");
                    }
                } else {    //TRANSMISIONES DISCOVERY->BOWMAN
                    if (strcmp(dBowman.upperInput, "LOGOUT") == 0) {
                        sig_func();
                    } else if (strcmp(dBowman.upperInput, "LIST SONGS") == 0) {
                        //creas msgqueue
                        //guardas el id devuelto en el array de queues (ints) --> 2
                        requestListSongs();
                        //TODO eliminas msgqueue cuando bowman out!!
                    } else if (strcmp(dBowman.upperInput, "LIST PLAYLISTS") == 0) {
                        requestListPlaylists();
                    } else if (strcmp(dBowman.upperInput, "CHECK DOWNLOADS") == 0) {
                        showDownloadStatus(dBowman.descargas, dBowman.numDescargas);
                    } else if (strcmp(dBowman.upperInput, "CLEAR DOWNLOADS") == 0) {
                        cleanThreadsBowman(&dBowman.descargas, &dBowman.numDescargas);
                        showDownloadStatus(dBowman.descargas, dBowman.numDescargas);
                    } else if (strstr(dBowman.upperInput, "DOWNLOAD") != NULL) {  //DOWNLOAD <SONG/PLAYLIST>
                        //comprobar 2 arguments --> 1 espai a la comanda
                        int numSpaces = checkDownloadCommand(dBowman.upperInput);
                        if (numSpaces == 1) {
                            //NUM ARGUMENTS CORRECTE!
                            int typeFile = songOrPlaylist(dBowman.upperInput);

                            char *nombreArchivoCopia = NULL;
                            char *nombreArchivo = strchr(dBowman.input, ' ');
                            if (nombreArchivo != NULL) {
                                size_t tamano = strlen(nombreArchivo + 1) + 1;
                                nombreArchivoCopia = malloc(tamano);
                                strcpy(nombreArchivoCopia, nombreArchivo + 1); // Copia desde el carácter después del espacio
                            }

                            if (typeFile == 1) {
                                requestDownloadSong(nombreArchivoCopia);
                            } else if (typeFile == 0) {
                                requestDownloadPlaylist(nombreArchivoCopia);
                            } else {
                                printF("ERROR: The song file extension is not valid.");
                            }
                        } else {
                            printF("Sorry number of arguments is not correct, try again\n");
                        }
                    } else {
                        printF("Unknown command\n");
                    }
                }
                freeString(&dBowman.input);
                freeString(&dBowman.upperInput);
            }
            close(fd);
            dBowman.msg = NULL;
            sig_func();
        }
    }
    return 0;
}