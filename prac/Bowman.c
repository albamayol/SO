/*
Autors:
    Alba Mayol Lozano -->alba.mayol
    Kevin Eljarrat Ohayon --> kevin.eljarrat
    LaSalle - Sistemes Operatius
Data creació: 15/10/23
Data última modificació: 16/5/24
*/

#include "Trama.h"

dataBowman dBowman;

typedef struct {
    long mtype;
    char type;
    short header_length;
    char header[256];
    char data[256];
} Missatge;

int requestLogout();
/*
@Finalitat: Inicialitzar les variables a NULL o al valor inicial desitjat.
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
    dBowman.maxDesc = 0;
    dBowman.infoPlaylists = NULL;
    dBowman.numInfoPlaylists = 0;
    dBowman.numDescargas = 0;
}

/*
@Finalitat: Gestiona la recepció de la signal (SIGINT) i allibera els recursos utilitzats fins el moment
@Paràmetres: ---
@Retorn: ---
*/
void sig_func() {
    //ESPERAMOS A QUE TERMINEN DE EJECUTARSE LOS THREADS 
    cleanAllTheThreadsBowman(&dBowman.descargas, dBowman.numDescargas); 

    if(dBowman.bowmanConnected) {  
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
    if (msgctl(dBowman.msgQueuePetitions, IPC_RMID, NULL) == -1) {
        perror("Error al eliminar la cola de mensajes");
    }

    // Eliminar cola de mensajes descargas canciones
    if (msgctl(dBowman.msgQueueDescargas, IPC_RMID, NULL) == -1) {
        perror("Error al eliminar la cola de mensajes");
    }

    cleanInfoPlaylists(dBowman.infoPlaylists, dBowman.numInfoPlaylists);

    close(dBowman.fdPoole);

    pthread_cancel(dBowman.threadRead);
    pthread_join(dBowman.threadRead, NULL);

    exit(EXIT_SUCCESS);
}

/*
@Finalitat: Printa la informació llegida de configBowman
@Paràmetres: ---
@Retorn: ---
*/
void printInfoFileBowman() {
    printF("\nFile read correctly:\n");
    asprintf(&dBowman.msg, "User - %s\nDirectory - %s\nIP - %s\nPort - %s\n\n", dBowman.clienteName, dBowman.pathClienteFile, dBowman.ip, dBowman.puerto);
    printF(dBowman.msg);
    freeString(&dBowman.msg);
}

/*
@Finalitat: Gestionar la connexió amb Discovery, obre socket i espera la trama amb la informació del Poole
@Paràmetres: ---
@Retorn: ---
*/
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

/*
@Finalitat: Mira si el Poole al que està connectat Bowman segueix o no "en peu" i avisa a Bowman de desconnectar-se també
@Paràmetres: ---
@Retorn: ---
*/
void checkPooleConnection() {
    dBowman.bowmanConnected = 0;
    asprintf(&dBowman.msg, "\n¡Alert: %s disconnected because the server connection has ended!\nPlease press Control C to exit the program.\n", dBowman.clienteName);
    printF(dBowman.msg);
    freeString(&dBowman.msg);
}

/*
@Finalitat: Funció de thread del fil de lectura, que rep trames, les neteja, les clasifica segons el tipus de trama i les envia a una cua de missatges o una altra
@Paràmetres: ---
@Retorn: ---
*/
static void *thread_function_read() {
    while(1) {
        Missatge msg;
        memset(&msg, 0, sizeof(Missatge));

        TramaExtended tramaExtended = readTrama(dBowman.fdPoole);

        if (tramaExtended.initialized) {
            freeTrama(&tramaExtended.trama);
            checkPooleConnection();
            return NULL;
        }

        msg.type = '\0';
        msg.type = tramaExtended.trama.type;
        msg.header_length = tramaExtended.trama.header_length;

        size_t i;
        memset(msg.header, '\0', 256);
        size_t sizeHeader = strlen(tramaExtended.trama.header);
        for (i = 0; i < sizeHeader; i++) {
            msg.header[i] = tramaExtended.trama.header[i];
        }
        
        memset(msg.data, '\0', 256);
        for (i = 0; i < 244; i++) {
            msg.data[i] = tramaExtended.trama.data[i];
        }
        
        if (strcmp(tramaExtended.trama.header, "FILE_DATA") == 0) { 
            char* stringID = read_until_string(tramaExtended.trama.data, '&'); //cribaje segun idsong
            msg.mtype = atoi(stringID);
            freeString(&stringID);

            if (msgsnd(dBowman.msgQueueDescargas, &msg, sizeof(Missatge) - sizeof(long), 0) == -1) { 
                perror("msgsnd"); 
                sig_func();
            }
        } else {
            if (strcmp(tramaExtended.trama.header, "CONOK") == 0 || strcmp(tramaExtended.trama.header, "CONKO") == 0) {                       //LOGOUT
                msg.mtype = 3;
            } else if (strcmp(tramaExtended.trama.header, "SONGS_RESPONSE") == 0) {                                                           //LIST SONGS
                msg.mtype = 1;
            } else if (strcmp(tramaExtended.trama.header, "PLAYLISTS_RESPONSE") == 0) {                                                       //LIST PLAYLISTS
                msg.mtype = 2;
            } else if (strcmp(tramaExtended.trama.header, "CON_OK") == 0 || strcmp(tramaExtended.trama.header, "CON_KO") == 0) {              //CONNECT POOLE
                msg.mtype = 7;
            } else if (strcmp(tramaExtended.trama.header, "PLAY_EXIST") == 0 || strcmp(tramaExtended.trama.header, "PLAY_NOEXIST") == 0) {    //CHECK PLAYLIST EXIST 
                msg.mtype = 5;
            } else if (strcmp(tramaExtended.trama.header, "FILE_NOEXIST") == 0 || strcmp(tramaExtended.trama.header, "FILE_EXIST") == 0) {    //CHECK SONG EXIST
                msg.mtype = 4;
            } else if (strcmp(tramaExtended.trama.header, "NEW_FILE") == 0) {                                                                 //NEW_FILE 
                msg.mtype = 6;
            }

            if (msgsnd(dBowman.msgQueuePetitions, &msg, sizeof(Missatge) - sizeof(long), 0) == -1) { 
                perror("msgsnd"); 
                sig_func();
            }
        } 
        freeTrama(&(tramaExtended.trama));
    }
    return NULL;
}

/*
@Finalitat: Crea el fil de lectura principal de trames
@Paràmetres: ---
@Retorn: ---
*/
void creacionHiloLectura() {
    if (pthread_create(&dBowman.threadRead, NULL, thread_function_read, NULL) != 0) {
        perror("Error al crear el thread de lectura\n");
    }
}

/*
@Finalitat: Gestionar la nova connexió amb el Poole i dona pas a la creació d'un fil principal de lectura de trames
@Paràmetres: ---
@Retorn: ---
*/
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
    
    Missatge msg;
    memset(&msg, 0, sizeof(Missatge));
    memset(msg.header, '\0', 256);
    memset(msg.data, '\0', 256);
    msg.type = '\0';
    
    if (msgrcv(dBowman.msgQueuePetitions, &msg, sizeof(Missatge) - sizeof(long), 7, 0) == -1) {
        printF("ERROR");
    }    
    
    if (strcmp(msg.header, "CON_OK") == 0) {
        dBowman.bowmanConnected = 1;
    } else if (strcmp(msg.header, "CON_KO") == 0) {
        close(dBowman.fdPoole);
    }
}

/*
@Finalitat: Junta les trames de cançons rebudes en un string.
@Paràmetres: int numTramas: número de trames a rebre; char **songs: cadena on es concatenaran les dades de les trames.
@Retorn: ---
*/
void juntarTramasSongs(int numTramas, char **songs) {
    int i = 0;
    size_t totalSize = 0; 
    Missatge msg;

    while(i < numTramas) {
        msgrcv(dBowman.msgQueuePetitions, &msg, sizeof(Missatge) - sizeof(long), 1, 0);
        
        cleanPadding(msg.data, '~');
        size_t dataSize = strlen(msg.data);
        *songs = realloc(*songs, totalSize + dataSize + 1);
        if (*songs == NULL) {
            break;
        }

        memcpy(*songs + totalSize, msg.data, dataSize);
        totalSize += dataSize;

        (*songs)[totalSize] = '\0';
        i++;
    }
}

/*
@Finalitat: Processa les trames de cançons per separar-les en una llista.
@Paràmetres: char ***canciones: array de strings on es guarden les cançons; char *songs: string amb les cançons concatenades.
@Retorn: El número de cançons processades.
*/
int procesarTramasSongs(char ***canciones, char *songs) {
    int numCanciones = 0, inicio = 0;

    char final = ' ', *song = NULL;

    do {
        song = readUntilFromIndex(songs, &inicio, '&', &final, '\0');

        *canciones = realloc(*canciones, (numCanciones + 1) * sizeof(char *));
        if (*canciones == NULL) {
            break;
        }
        (*canciones)[numCanciones] = song;
        numCanciones++;
    } while (final != '\0');
    return numCanciones;
}

/*
@Finalitat: Printa les cançons per pantalla.
@Paràmetres: int numCanciones: número de cançons; char ***canciones: llista de strings amb les cançons a imprimir.
@Retorn: ---
*/
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

/*
@Finalitat: Gestiona la petició d'una llista de les cançons de Poole, envia la petició i processa la resposta.
@Paràmetres: ---
@Retorn: ---
*/
void requestListSongs() {
    int numCanciones = 0;
    char *songs = NULL, **canciones = NULL;

    setTramaString(TramaCreate(0x02, "LIST_SONGS", "", 0), dBowman.fdPoole);

    Missatge msg;
    msgrcv(dBowman.msgQueuePetitions, &msg, sizeof(Missatge) - sizeof(long), 1, 0);

    cleanPadding(msg.data, '~');
    int numTramas = atoi(msg.data);

    juntarTramasSongs(numTramas, &songs);

    numCanciones = procesarTramasSongs(&canciones, songs);

    printarSongs(numCanciones, &canciones);

    free(canciones);
    freeString(&songs);
}

/*
@Finalitat: Junta les trames rebudes de Poole envers la llista de playlists de Poole
@Paràmetres: int numTramas: número de trames a rebre
@Retorn: char*: 
*/
char *juntarTramasPlaylists(int numTramas) {
    int i = 0;
    char *playlists = NULL;
    ssize_t totalSize = 0; 
    Missatge msg;

    while(i < numTramas) {

        msgrcv(dBowman.msgQueuePetitions, &msg, sizeof(Missatge) - sizeof(long), 2, 0);

        cleanPadding(msg.data, '~');
        int dataSize = strlen(msg.data);

        playlists = realloc(playlists, totalSize + dataSize + 1);
        if (playlists == NULL) {
            break;
        }

        memcpy(playlists + totalSize, msg.data, dataSize);
        totalSize += dataSize;

        playlists[totalSize] = '\0';
        i++;
    }

    size_t len = strlen(playlists);
    playlists = realloc(playlists, len + 2);
    if (playlists != NULL) {
        playlists[len] = '#';
        playlists[len + 1] = '\0';
    }
    return playlists;
}

/*
@Finalitat: Processa les trames de cançons i les guarda a una llista de strings.
@Paràmetres: char ***canciones: array de strings on es guardaran les cançons processades; char *songs: string amb les cançons concatenades.
@Retorn: Nombre de cançons processades.
*/
char ***procesarTramasPlaylists(char *playlists, int **numCancionesPorLista, int numCanciones, int *numListas) {
    int i = 0, totalCanciones = 0, inicioPlaylist = 0, inicioSong = 0;
    char valorFinal = ' ', ***listas = NULL, *playlist = NULL, *song = NULL;

    do {
        playlist = readUntilFromIndex(playlists, &inicioPlaylist, '#', &valorFinal, '\0');

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

                listas[*numListas] = malloc(sizeof(char *));
                if (listas[*numListas] == NULL) {
                    break;
                }
                
                listas[*numListas][i] = strdup(song); 
                if (listas[*numListas][i] == NULL) {
                    free(listas[*numListas]);
                    break;
                }
            } else {
                listas[*numListas] = realloc(listas[*numListas], (i + 1) * sizeof(char *));
                if (listas[*numListas] == NULL) {
                    break;
                }

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

/*
@Finalitat: Calcula el número de cançons que té una playlist en específic
@Paràmetres: InfoPlaylist* infoPlaylists: array de les playlists; char* listName: nom de la playlist
@Retorn: int: número de cançons d'una playlist
*/
int numSongsDePlaylist(InfoPlaylist* infoPlaylists, char* listName) {
    for (int i = 0; i < dBowman.numInfoPlaylists; i++) {
        if (strcmp(infoPlaylists[i].nameplaylist, listName) == 0) {
            return infoPlaylists[i].numSongs; 
        }
    }
    return -1;
}

/*
@Finalitat: Printa la llista de playlists de Poole
@Paràmetres: int numListas: número de playlists que té Poole; char*** listas: llista de les playlists on cada playlist conté una llista de les seves cançons; int* numCancionesPorLista: número de cançons que té cada playlist
@Retorn: ---
*/
void printarPlaylists(int numListas, char ***listas, int *numCancionesPorLista) {
    asprintf(&dBowman.msg, "\nThere are %d lists available for download:", numListas);
    printF(dBowman.msg);
    freeString(&dBowman.msg);

    for (int i = 0; i < numListas; i++) {
        asprintf(&dBowman.msg, "\n%d. %s", i + 1, listas[i][0]);
        printF(dBowman.msg);
        freeString(&dBowman.msg);

        // Guardamos info Playlists actuales
        dBowman.infoPlaylists = realloc(dBowman.infoPlaylists, sizeof(InfoPlaylist) * (dBowman.numInfoPlaylists + 1));
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

/*
@Finalitat: Gestiona la petició i recepció de llistar les playlists de Poole
@Paràmetres: ---
@Retorn: ---
*/
void requestListPlaylists() {
    char *playlists = NULL, ***listas = NULL; 
    int numListas = 0;
    int *numCancionesPorLista = malloc(sizeof(int)); 
    *numCancionesPorLista = 0;
    Missatge msg;

    setTramaString(TramaCreate(0x02, "LIST_PLAYLISTS", "", 0), dBowman.fdPoole);

    // Lectura cantidad de canciones
    msgrcv(dBowman.msgQueuePetitions, &msg, sizeof(Missatge) - sizeof(long), 2, 0);

    cleanPadding(msg.data, '~');
    int numCanciones = atoi(msg.data);

    msgrcv(dBowman.msgQueuePetitions, &msg, sizeof(Missatge) - sizeof(long), 2, 0);

    int numTramas = atoi(msg.data);

    playlists = juntarTramasPlaylists(numTramas);

    listas = procesarTramasPlaylists(playlists, &numCancionesPorLista, numCanciones, &numListas);
    
    printarPlaylists(numListas, listas, numCancionesPorLista);

    free(playlists);
}

/*
@Finalitat: Gestiona la desconnexió del propi Bowman amb Poole, envia una trama avisant-lo
@Paràmetres: ---
@Retorn: ---
*/
int requestLogout() {  
    setTramaString(TramaCreate(0x06, "EXIT", dBowman.clienteName, strlen(dBowman.clienteName)), dBowman.fdPoole);

    Missatge msg;

    msgrcv(dBowman.msgQueuePetitions, &msg, sizeof(Missatge) - sizeof(long), 3, 0); 

    if (strcmp(msg.header, "CONOK") == 0) {
        printF(msg.header);
        close(dBowman.fdPoole);
        return 1;
    } else if (strcmp(msg.header, "CONKO")) {
        printF(msg.header);
        printF("Sorry, couldn't logout, try again\n");
        return 0;
    }
    return 2;
}

/*
@Finalitat: Separa el identificador i els bytes del camp data de la trama de descarrega d'una cançó
@Paràmetres: char* buffer: camp data de la trama; char* dataFile: contindrà els bytes del camp data (netejats sense el id i el &); DescargaBowman* mythread: estructura amb les dades necessaries per a la descarrega
@Retorn: ---
*/
void getIdData(char* buffer, char* dataFile, DescargaBowman *mythread) { 
    int counter = 0, i = 0; 

    //saltamos id
    while (buffer[counter] != '&') {
        counter++;
    }
    counter++; //saltamos &

    while ((counter < 244) && (mythread->song.bytesDescargados < mythread->song.size)) {
        dataFile[i] = buffer[counter];
        i++;
        counter++;

        mythread->song.bytesDescargados++;
    }
}

/*
@Finalitat: Crea el fitxer .mp3 al path concret i escriu els bytes que li envia Poole per a aquella cançó.
@Paràmetres: char* directory: path al directori on es crearà el .mp3; DescargaBowman* mythread: estructura amb les dades necessaries per a la descarrega; size_t size: mida de la cançó; int id_song: identificador de la cançó
@Retorn: ---
*/
void createMP3FileInDirectory(char* directory, DescargaBowman *mythread, size_t size, int idSong) {
    char dataFile[244]; 
    memset(&dataFile, '\0', 244);

    char *path = NULL;

    dBowman.descargas[mythread->index].nombreCancion = strdup(mythread->song.nombre);
    if (strcmp(mythread->song.nombre, mythread->nombreDescargaComando) == 0) {
        //cancion
        size_t len = strlen(directory) + strlen(mythread->song.nombre) + 2;
        path = malloc(len);
        snprintf(path, len, "%s/%s", directory, mythread->song.nombre); 

        dBowman.descargas[mythread->index].nombrePlaylist = NULL; 
    } else {
        //playlist
        size_t len = strlen(directory) + strlen(mythread->nombreDescargaComando) + strlen(mythread->song.nombre) + 3;
        path = malloc(len);
        snprintf(path, len, "%s/%s/%s", directory, mythread->nombreDescargaComando, mythread->song.nombre); 

        dBowman.descargas[mythread->index].nombrePlaylist = strdup(mythread->nombreDescargaComando); 
    }

    int fd_file = open(path, O_CREAT | O_RDWR, 0644); 
    if (fd_file == -1) {
        perror("Error al crear el archivo");
        freeString(&path);
        sig_func();
    }

    int file_size = size;
    char* idSongString = convertIntToString(idSong);
    int sizeDataTrama = 256 - 12 - strlen(idSongString) - 1;
    freeString(&idSongString);

    do {
        Missatge msg;
        
        msgrcv(dBowman.msgQueueDescargas, &msg, sizeof(Missatge) - sizeof(long), idSong, 0);

        getIdData(msg.data, dataFile, mythread);

        if (write(fd_file, dataFile, min(file_size, sizeDataTrama)) == -1) { 
            perror("Error al escribir en el archivo");
            break;
        }

        file_size -= sizeDataTrama;

        float porcentaje = ((float)mythread->song.bytesDescargados / mythread->song.size) * 100;

        dBowman.descargas[mythread->index].porcentaje = porcentaje;
    } while (file_size > 0); 
    
    char *md5sum = resultMd5sumComand(path);
    if (md5sum != NULL) {
        if (strcmp(md5sum, mythread->song.md5sum) == 0) {
            setTramaString(TramaCreate(0x05, "CHECK_OK", dBowman.descargas[mythread->index].nombreCancion, strlen(dBowman.descargas[mythread->index].nombreCancion)), dBowman.fdPoole);
        } else {
            setTramaString(TramaCreate(0x05, "CHECK_KO", "", 0), dBowman.fdPoole);
        }
        freeString(&md5sum);
    }
    
    close(fd_file);
    freeString(&path);
}

/*
@Finalitat: Guarda la informació rebuda de la trama inicial enviada pel Poole i dona pas a la descarrega dels bytes d'aquella cançó
@Paràmetres: DescargaBowman* mythread: estructura amb les dades necessaries per a la descarrega
@Retorn: ---
*/
void downloadSong(DescargaBowman *mythread) {
    char valorFinal = ' ';
    int inicio = 0, i = 1;
    Missatge msg;

    msgrcv(dBowman.msgQueuePetitions, &msg, sizeof(Missatge) - sizeof(long), 6, 0);

    cleanPadding(msg.data, '~');
    char *dataSong = strdup(msg.data); 

    while (valorFinal != '\0') {
        char *paramDataSong = readUntilFromIndex(dataSong, &inicio, '&', &valorFinal, '\0');
        switch (i) {
            case 1: 
                mythread->song.nombre = strdup(paramDataSong);
                break;
            case 2: 
                mythread->song.size = atoi(paramDataSong);
                break;
            case 3: 
                mythread->song.md5sum = strdup(paramDataSong); 
                break;
            case 4: 
                mythread->song.id = atoi(paramDataSong);
                break;
        }
        freeString(&paramDataSong);
        i++;
    }    
    freeString(&dataSong);

    createMP3FileInDirectory(dBowman.clienteName, mythread, mythread->song.size, mythread->song.id);
}

/*
@Finalitat: Funció de thread per a la descarrega d'una cançó
@Paràmetres: void* thread: estructura de la descarrega amb la informació necessària
@Retorn: ---
*/
static void *thread_function_download_song(void* thread) {
    DescargaBowman *mythread = (DescargaBowman*) thread; 
    dBowman.maxDesc++;

    dBowman.descargas[mythread->index].thread_id = pthread_self(); 
    dBowman.descargas[mythread->index].porcentaje = 0.00;

    downloadSong(mythread);

    freeString(&(mythread->nombreDescargaComando));
    freeString(&(mythread->song.nombre));
    freeString(&(mythread->song.md5sum));
    free(mythread); 

    return NULL; 
}

/*
@Finalitat: Crea nova posició a l'array de descarregues de Bowman i un nou thread per a una cançó
@Paràmetres: char* song: nom de la cançó que es descarregarà; int index: index de la descarrega dins l'array de descarregues de Bowman
@Retorn: ---
*/
void threadDownloadSong(char *song, int index) {   
    pthread_t thread;
    DescargaBowman *db = malloc(sizeof(DescargaBowman));

    db->nombreDescargaComando = strdup(song);
    db->song.bytesDescargados = 0; 
    db->index = index;

    if (pthread_create(&thread, NULL, thread_function_download_song, (void *)db) != 0) {
        perror("Error al crear el thread de descarga\n");
    }
}

/*
@Finalitat: Gestiona la petició de descarrega d'una cançó, comprova si és possible la seva descarrega i genera un nou thread per a la cançó
@Paràmetres: char* nombreArchivoCopia: nom de la cançó que es vol descarregar
@Retorn: ---
*/
void requestDownloadSong(char* nombreArchivoCopia) {
    if (dBowman.maxDesc < 3) {
        setTramaString(TramaCreate(0x03, "DOWNLOAD_SONG", nombreArchivoCopia, strlen(nombreArchivoCopia)), dBowman.fdPoole); //playlistname / songname
        //ESPERAMOS TRAMA SI SONG EXISTE O NO
        Missatge msg;
        msgrcv(dBowman.msgQueuePetitions, &msg, sizeof(Missatge) - sizeof(long), 4, 0);

        if (strcmp(msg.header, "FILE_EXIST") == 0) {
            printF("Download started!");
            dBowman.descargas = realloc(dBowman.descargas, (dBowman.numDescargas + 1) * sizeof(Descarga));
            threadDownloadSong(nombreArchivoCopia, dBowman.numDescargas);
            dBowman.numDescargas++;
            freeString(&nombreArchivoCopia);
        } else if (strcmp(msg.header, "FILE_NOEXIST") == 0) {
        }    
    } else {
        asprintf(&dBowman.msg, "Error al descargar %s. Intentalo cuando finalizen las descargas actuales. Y realiza un 'Clear downloads'\n", nombreArchivoCopia);
        perror(dBowman.msg);
        freeString(&dBowman.msg);
    }
}

/*
@Finalitat: Gestiona la petició de descarrega d'una playlist, comprova si és possible la seva descarrega i genera un nou thread per a cada cançó
@Paràmetres: char* nombreArchivoCopia: nom de la playlist que es vol descarregar
@Retorn: ---
*/
void requestDownloadPlaylist(char* nombreArchivoCopia) {
    if (dBowman.maxDesc < 3) {
        setTramaString(TramaCreate(0x03, "DOWNLOAD_LIST", nombreArchivoCopia, strlen(nombreArchivoCopia)), dBowman.fdPoole); 

        //ESPERAMOS TRAMA SI PLAYLIST EXISTE O NO
        Missatge msg;
        msgrcv(dBowman.msgQueuePetitions, &msg, sizeof(Missatge) - sizeof(long), 5, 0);

        if (strcmp(msg.header, "PLAY_EXIST") == 0) {
            printF("Download started!\n");
            char *playlistDirectory = malloc(strlen(dBowman.clienteName) + strlen(nombreArchivoCopia) + 2); 
            sprintf(playlistDirectory, "%s/%s", dBowman.clienteName, nombreArchivoCopia);   
            createDirectory(playlistDirectory);
            freeString(&playlistDirectory);

            int numSongs = numSongsDePlaylist(dBowman.infoPlaylists, nombreArchivoCopia);

            dBowman.descargas = realloc(dBowman.descargas, (dBowman.numDescargas + numSongs) * sizeof(Descarga));
        
            for (int i = 0; i < numSongs; i++) {
                threadDownloadSong(nombreArchivoCopia, dBowman.numDescargas + i);
            }
            freeString(&nombreArchivoCopia);
            dBowman.numDescargas += numSongs;
        } else if (strcmp(msg.header, "PLAY_NOEXIST") == 0) {
            printF("This playlist does not exist.\n");
        }
    } else {
        asprintf(&dBowman.msg, "Error al descargar %s. Intentalo cuando finalizen las descargas actuales. Y realiza un 'Clear downloads'\n", nombreArchivoCopia);
        printF(dBowman.msg);
        freeString(&dBowman.msg);
    }
}

/*
@Finalitat: Crea 2 cues de missatges per a la gestió de les trames
@Paràmetres: ---
@Retorn: ---
*/
void creacionMsgQueues() {
    key_t key1 = ftok("Bowman.c", 0xCA);

    int id_queue = msgget(key1, 0666 | IPC_CREAT);
    if (id_queue < 0) {
        write(1, "Error al crear la cua de missatges de les peticions\n", strlen("Error al crear la cua de missatges de les peticions\n"));
        return;
    }
    dBowman.msgQueuePetitions = id_queue;

    key_t key2 = ftok("Bowman.c", 0xCB);

    id_queue = msgget(key2, 0666 | IPC_CREAT);
    if (id_queue < 0) {
        write(1, "Error al crear la cua de missatges de les descargues\n", strlen("Error al crear la cua de missatges de les descargues\n"));
        return;
    }
    dBowman.msgQueueDescargas = id_queue;
}

/*
@Finalitat: Mostra el percentatge actual de les descarregues en curs
@Paràmetres: Descarga* descargas: array de les descarregues de Bowman
@Retorn: ---
*/
void showDownloadStatus(Descarga *descargas) {
    for (int i = 0; i < dBowman.numDescargas; i++) {
        if (descargas[i].nombreCancion != NULL) {
            // Descarga no eliminada
            if (descargas[i].nombrePlaylist == NULL) {
                //Es una cancion
                printF(descargas[i].nombreCancion);
                printF("\n");
            } else {
                // Es una cancion de una playlist
                asprintf(&dBowman.msg, "%s - %s\n", descargas[i].nombrePlaylist, descargas[i].nombreCancion);
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
@Finalitat: Implementar el main del procès Bowman, gestiona les peticions de l'usuari desde la línia de comandes.
@Paràmetres: int argc: número d'arguments, char** argv array amb els arguments del programa.
@Retorn: int: 0 en cas de que el programa hagi finalitzat exitosament.
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

            close(fd);

            asprintf(&dBowman.msg, "\n%s user initialized\n", dBowman.clienteName);
            printF(dBowman.msg);
            freeString(&dBowman.msg);

            createDirectory(dBowman.clienteName);

            printInfoFileBowman();

            creacionMsgQueues();

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
                        requestListSongs();
                    } else if (strcmp(dBowman.upperInput, "LIST PLAYLISTS") == 0) {
                        requestListPlaylists();
                    } else if (strcmp(dBowman.upperInput, "CHECK DOWNLOADS") == 0) {
                        showDownloadStatus(dBowman.descargas);
                    } else if (strcmp(dBowman.upperInput, "CLEAR DOWNLOADS") == 0) {
                        cleanThreadsBowman(&dBowman.descargas, &dBowman.numDescargas, &dBowman.maxDesc);
                        showDownloadStatus(dBowman.descargas);
                    } else if (strstr(dBowman.upperInput, "DOWNLOAD") != NULL) { 
                        int numSpaces = checkDownloadCommand(dBowman.upperInput);
                        if (numSpaces == 1) {
                            int typeFile = songOrPlaylist(dBowman.upperInput);

                            char *nombreArchivoCopia = NULL;
                            char *nombreArchivo = strchr(dBowman.input, ' ');

                            if (nombreArchivo != NULL) {
                                size_t tamano = strlen(nombreArchivo + 1) + 1;
                                nombreArchivoCopia = malloc(tamano);
                                strcpy(nombreArchivoCopia, nombreArchivo + 1);
                            }

                            if (typeFile == 1) {
                                requestDownloadSong(nombreArchivoCopia);
                            } else if (typeFile == 0) {
                                requestDownloadPlaylist(nombreArchivoCopia);
                            } else {
                                printF("ERROR: The song file extension is not valid.\n");
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
        }
    }
    return 0;
}