/*
Autors:
    Alba Mayol Lozano -->alba.mayol
    Kevin Eljarrat Ohayon --> kevin.eljarrat
    LaSalle - Sistemes Operatius
Data creació: 15/10/23
Data última modificació: 16/5/24
*/
#include "Trama.h"

typedef struct {
    int id;
    char *md5sum;
    char *nombre;
    size_t size;
    size_t bytesDescargados;
} Song;

typedef struct {
    Song song; 
    char *nombreDescargaComando; 
    int index; 
} DescargaBowman;

typedef struct {
    long mtype;
    char type;
    short header_length;
    char header[256];
    char data[256];
} Missatge;

typedef struct {
    char *nombreCancion;	    
    char *nombrePlaylist;
    float porcentaje;   
    pthread_t thread_id;
} Descarga;

int fdDiscovery;
int fdPoole;
struct sockaddr_in discovery_addr;
struct sockaddr_in poole_addr;
int bowmanConnected;
Element pooleConnected;
pthread_t threadRead;
InfoPlaylist* infoPlaylists;
int numInfoPlaylists;
Descarga *descargas;
int numDescargas;
int maxDesc;
int msgQueuePetitions;
int msgQueueDescargas;
char *msgAuxiliar;

int requestLogout();
/*
@Finalitat: Inicialitzar les variables a NULL o al valor inicial desitjat.
@Paràmetres: ---
@Retorn: ---
*/
void inicializarDataBowman() {
    initScreen();
    msgAuxiliar = NULL;
    clienteNameAux = NULL;
    clienteName = NULL;
    pathClienteFile = NULL;
    ip = NULL;
    puerto = NULL;
    bowmanConnected = 0;
    maxDesc = 0;
    infoPlaylists = NULL;
    numInfoPlaylists = 0;
    numDescargas = 0;
}

/*
@Finalitat: cancelar i matar tots els threads de Bowman 
@Paràmetres: Descarga** descargas: array de descarrgues de Bowman; int* numDescargas: mida de l'array de descarregues;
@Retorn: ---
*/
void cleanAllTheThreadsBowman(Descarga **descargas, int numDescargas) {
    if (numDescargas != 0) {
        write(1, "\nYou are waiting for the completion of the downloads of the songs in progress to exit the program.\n", strlen("\nYou are waiting for the completion of the downloads of the songs in progress to exit the program.\n"));
    }

    for (int i = 0; i < numDescargas; i++) {
        if ((*descargas)[i].nombreCancion != NULL) {
            pthread_join((*descargas)[i].thread_id, NULL);
            free((*descargas)[i].nombreCancion);
            (*descargas)[i].nombreCancion = NULL;
            free((*descargas)[i].nombrePlaylist);
            (*descargas)[i].nombrePlaylist = NULL;
        }
    }
    free(*descargas);
}

/*
@Finalitat: cancelar i matar els threads de Bowman que ja hagin acabat les descarregues
@Paràmetres: Descarga** descargas: array de descarrgues de Bowman; int* numDescargas: mida de l'array de descarregues; int* maxDesc: punter al número máxim de descarregues possibles
@Retorn: ---
*/
void cleanThreadsBowman(Descarga **descargas, int *numDescargas, int *maxDesc) { 
    int numDescargasAux = *numDescargas;
    *maxDesc = 0;

    for (int i = 0; i < numDescargasAux; i++) {
        if ((*descargas)[i].porcentaje == 100) {
            pthread_cancel((*descargas)[i].thread_id);
            pthread_join((*descargas)[i].thread_id, NULL);
            free((*descargas)[i].nombrePlaylist);
            (*descargas)[i].nombrePlaylist = NULL;
            free((*descargas)[i].nombreCancion);
            (*descargas)[i].nombreCancion = NULL;
        } 
    }
}

/*
@Finalitat: Neteja i allibera l'array de playlists que rep Bowman segons la llista de playlists del seu Poole
@Paràmetres: InfoPlaylist* infoPlaylists: array de playlists de Bowman; int size: mida de l'array de playlists
@Retorn: ---
*/
void cleanInfoPlaylists(InfoPlaylist *infoPlaylists, int size) {
    for (int i = 0; i < size; ++i) {
        free(infoPlaylists[i].nameplaylist);
        infoPlaylists[i].nameplaylist = NULL;
    }
    free(infoPlaylists);
}

/*
@Finalitat: Gestiona la recepció de la signal (SIGINT) i allibera els recursos utilitzats fins el moment
@Paràmetres: ---
@Retorn: ---
*/
void sig_func() {
    cleanAllTheThreadsBowman(&descargas, numDescargas); 
    if(bowmanConnected) {  
        if (requestLogout()) {
            printF("Thanks for using HAL 9000, see you soon, music lover!\n");
        } 
    }
    if(msgAuxiliar != NULL) {
        freeString(&msgAuxiliar);
    }
    if(clienteName != NULL) {
        freeString(&clienteName);
    }
    if(clienteNameAux != NULL) {
        freeString(&clienteNameAux);
    }
    if(pathClienteFile != NULL) {
        freeString(&pathClienteFile);
    }
    if(ip != NULL) {
        freeString(&ip);
    }
    if(puerto != NULL) {
        freeString(&puerto);
    }
    freeElement(&pooleConnected);

    if (msgctl(msgQueuePetitions, IPC_RMID, NULL) == -1) {
        perror("Error al eliminar la cola de mensajes");
    }
    if (msgctl(msgQueueDescargas, IPC_RMID, NULL) == -1) {
        perror("Error al eliminar la cola de mensajes");
    }
    cleanInfoPlaylists(infoPlaylists, numInfoPlaylists);
    close(fdPoole);
    pthread_cancel(threadRead);
    pthread_join(threadRead, NULL);
    destroyMutexScreen();
    exit(EXIT_SUCCESS);
}

/*
@Finalitat: Gestionar la connexió amb Discovery, obre socket i espera la trama amb la informació del Poole
@Paràmetres: ---
@Retorn: ---
*/
void establishDiscoveryConnection() {
    fdDiscovery = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (fdDiscovery < 0) {
        perror ("Error al crear el socket de Discovery");
        close(fdDiscovery);
        sig_func();
    }

    bzero (&discovery_addr, sizeof (discovery_addr));
    discovery_addr.sin_family = AF_INET;
    discovery_addr.sin_port = htons(atoi(puerto)); 
    discovery_addr.sin_addr.s_addr = inet_addr(ip);
    if (connect(fdDiscovery, (struct sockaddr*)&discovery_addr, sizeof(discovery_addr)) < 0) {
        perror("Error al conectar a Discovery");
        close(fdDiscovery);
        sig_func();
    }

    //TRANSMISIONES DISCOVERY->BOWMAN
    char *aux = NULL;
    int length = strlen(clienteName) + 3;
    aux = (char *) malloc(sizeof(char) * length);
    for (int i = 0; i < length; i++) {
        aux[i] = '\0';
    }
    strcpy(aux, clienteName);
    setTramaString(TramaCreate(0x01, "NEW_BOWMAN", aux, strlen(aux)), fdDiscovery);
    freeString(&aux);

    TramaExtended tramaExtended = readTrama(fdDiscovery);
    if (strcmp(tramaExtended.trama.header,"CON_OK") == 0)  {
        separaDataToElement(tramaExtended.trama.data, &pooleConnected);
        asprintf(&msgAuxiliar, "%s connected to HAL 9000 system, welcome music lover!\n", clienteName);
        printF(msgAuxiliar);
        freeString(&msgAuxiliar);
    } else if (strcmp(tramaExtended.trama.header,"CON_KO") == 0) {
        write(1, "CON_KO\n", strlen("CON_KO\n"));
    }
    freeTrama(&(tramaExtended.trama));
    close(fdDiscovery);
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
        TramaExtended tramaExtended = readTrama(fdPoole);
        if (tramaExtended.initialized) {
            freeTrama(&tramaExtended.trama);
            checkPooleConnection(&bowmanConnected, clienteName);
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
            if (msgsnd(msgQueueDescargas, &msg, sizeof(Missatge) - sizeof(long), 0) == -1) { 
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
            if (msgsnd(msgQueuePetitions, &msg, sizeof(Missatge) - sizeof(long), 0) == -1) { 
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
    if (pthread_create(&threadRead, NULL, thread_function_read, NULL) != 0) {
        perror("Error al crear el thread de lectura\n");
    }
}

/*
@Finalitat: Gestionar la nova connexió amb el Poole i dona pas a la creació d'un fil principal de lectura de trames
@Paràmetres: ---
@Retorn: ---
*/
void establishPooleConnection() {
    fdPoole = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (fdPoole < 0) {
        perror ("Error al crear el socket de Poole");
        close(fdPoole);
        sig_func();
    }

    bzero (&poole_addr, sizeof (poole_addr));
    poole_addr.sin_family = AF_INET;
    poole_addr.sin_port = htons(pooleConnected.port); 
    poole_addr.sin_addr.s_addr = inet_addr(pooleConnected.ip);
    if (connect(fdPoole, (struct sockaddr*)&poole_addr, sizeof(poole_addr)) < 0) {
        perror("Error al conectar a Poole");
        close(fdPoole);
        sig_func();
    }
    creacionHiloLectura();

    // Transmission Bowman->Poole
    setTramaString(TramaCreate(0x01, "NEW_BOWMAN", clienteName, strlen(clienteName)), fdPoole);
    Missatge msg;
    memset(&msg, 0, sizeof(Missatge));
    memset(msg.header, '\0', 256);
    memset(msg.data, '\0', 256);
    msg.type = '\0';
    
    if (msgrcv(msgQueuePetitions, &msg, sizeof(Missatge) - sizeof(long), 7, 0) == -1) {
        printF("ERROR");
    }    
    if (strcmp(msg.header, "CON_OK") == 0) {
        bowmanConnected = 1;
    } else if (strcmp(msg.header, "CON_KO") == 0) {
        close(fdPoole);
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
        msgrcv(msgQueuePetitions, &msg, sizeof(Missatge) - sizeof(long), 1, 0);
        
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
@Finalitat: Gestiona la petició d'una llista de les cançons de Poole, envia la petició i processa la resposta.
@Paràmetres: ---
@Retorn: ---
*/
void requestListSongs() {
    int numCanciones = 0;
    char *songs = NULL, **canciones = NULL;
    setTramaString(TramaCreate(0x02, "LIST_SONGS", "", 0), fdPoole);

    Missatge msg;
    msgrcv(msgQueuePetitions, &msg, sizeof(Missatge) - sizeof(long), 1, 0);
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
        msgrcv(msgQueuePetitions, &msg, sizeof(Missatge) - sizeof(long), 2, 0);
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
@Finalitat: Calcula el número de cançons que té una playlist en específic
@Paràmetres: InfoPlaylist* infoPlaylists: array de les playlists; char* listName: nom de la playlist
@Retorn: int: número de cançons d'una playlist
*/
int numSongsDePlaylist(InfoPlaylist* infoPlaylists, char* listName) {
    for (int i = 0; i < numInfoPlaylists; i++) {
        if (strcmp(infoPlaylists[i].nameplaylist, listName) == 0) {
            return infoPlaylists[i].numSongs; 
        }
    }
    return -1;
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

    setTramaString(TramaCreate(0x02, "LIST_PLAYLISTS", "", 0), fdPoole);
    msgrcv(msgQueuePetitions, &msg, sizeof(Missatge) - sizeof(long), 2, 0); // Lectura cantidad de canciones
    cleanPadding(msg.data, '~');
    int numCanciones = atoi(msg.data);

    msgrcv(msgQueuePetitions, &msg, sizeof(Missatge) - sizeof(long), 2, 0);
    int numTramas = atoi(msg.data);

    playlists = juntarTramasPlaylists(numTramas);
    listas = procesarTramasPlaylists(playlists, &numCancionesPorLista, numCanciones, &numListas);
    printarPlaylists(numListas, listas, numCancionesPorLista, &infoPlaylists, &numInfoPlaylists);
    free(playlists);
}

/*
@Finalitat: Gestiona la desconnexió del propi Bowman amb Poole, envia una trama avisant-lo
@Paràmetres: ---
@Retorn: ---
*/
int requestLogout() {  
    setTramaString(TramaCreate(0x06, "EXIT", clienteName, strlen(clienteName)), fdPoole);
    Missatge msg;
    msgrcv(msgQueuePetitions, &msg, sizeof(Missatge) - sizeof(long), 3, 0); 

    if (strcmp(msg.header, "CONOK") == 0) {
        printF(msg.header);
        close(fdPoole);
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
    while (buffer[counter] != '&') { //saltamos id
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

    descargas[mythread->index].nombreCancion = strdup(mythread->song.nombre);
    if (strcmp(mythread->song.nombre, mythread->nombreDescargaComando) == 0) { //cancion
        size_t len = strlen(directory) + strlen(mythread->song.nombre) + 2;
        path = malloc(len);
        snprintf(path, len, "%s/%s", directory, mythread->song.nombre); 
        descargas[mythread->index].nombrePlaylist = NULL; 
    } else { //playlist
        size_t len = strlen(directory) + strlen(mythread->nombreDescargaComando) + strlen(mythread->song.nombre) + 3;
        path = malloc(len);
        snprintf(path, len, "%s/%s/%s", directory, mythread->nombreDescargaComando, mythread->song.nombre); 
        descargas[mythread->index].nombrePlaylist = strdup(mythread->nombreDescargaComando); 
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
        msgrcv(msgQueueDescargas, &msg, sizeof(Missatge) - sizeof(long), idSong, 0);
        getIdData(msg.data, dataFile, mythread);
        if (write(fd_file, dataFile, min(file_size, sizeDataTrama)) == -1) { 
            perror("Error al escribir en el archivo");
            break;
        }
        file_size -= sizeDataTrama;
        
        float porcentaje = ((float)mythread->song.bytesDescargados / mythread->song.size) * 100;
        descargas[mythread->index].porcentaje = porcentaje;
    } while (file_size > 0); 
    char *md5sum = resultMd5sumComand(path);
    if (md5sum != NULL) {
        if (strcmp(md5sum, mythread->song.md5sum) == 0) {
            setTramaString(TramaCreate(0x05, "CHECK_OK", descargas[mythread->index].nombreCancion, strlen(descargas[mythread->index].nombreCancion)), fdPoole);
        } else {
            setTramaString(TramaCreate(0x05, "CHECK_KO", "", 0), fdPoole);
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

    msgrcv(msgQueuePetitions, &msg, sizeof(Missatge) - sizeof(long), 6, 0);
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
    createMP3FileInDirectory(clienteName, mythread, mythread->song.size, mythread->song.id);
}

/*
@Finalitat: Funció de thread per a la descarrega d'una cançó
@Paràmetres: void* thread: estructura de la descarrega amb la informació necessària
@Retorn: ---
*/
static void *thread_function_download_song(void* thread) {
    DescargaBowman *mythread = (DescargaBowman*) thread; 
    maxDesc++;
    descargas[mythread->index].thread_id = pthread_self(); 
    descargas[mythread->index].porcentaje = 0.00;
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
    if (maxDesc < 3) {
        setTramaString(TramaCreate(0x03, "DOWNLOAD_SONG", nombreArchivoCopia, strlen(nombreArchivoCopia)), fdPoole); //playlistname / songname

        Missatge msg;
        msgrcv(msgQueuePetitions, &msg, sizeof(Missatge) - sizeof(long), 4, 0); //ESPERAMOS TRAMA SI SONG EXISTE O NO
        if (strcmp(msg.header, "FILE_EXIST") == 0) {
            printF("Download started!");
            descargas = realloc(descargas, (numDescargas + 1) * sizeof(Descarga));
            threadDownloadSong(nombreArchivoCopia, numDescargas);
            numDescargas++;
            freeString(&nombreArchivoCopia);
        } else if (strcmp(msg.header, "FILE_NOEXIST") == 0) {
        }    
    } else {
        asprintf(&msgAuxiliar, "Error al descargar %s. Intentalo cuando finalizen las descargas actuales. Y realiza un 'Clear downloads'\n", nombreArchivoCopia);
        perror(msgAuxiliar);
        freeString(&msgAuxiliar);
    }
}

/*
@Finalitat: Gestiona la petició de descarrega d'una playlist, comprova si és possible la seva descarrega i genera un nou thread per a cada cançó
@Paràmetres: char* nombreArchivoCopia: nom de la playlist que es vol descarregar
@Retorn: ---
*/
void requestDownloadPlaylist(char* nombreArchivoCopia) {
    if (maxDesc < 3) {
        setTramaString(TramaCreate(0x03, "DOWNLOAD_LIST", nombreArchivoCopia, strlen(nombreArchivoCopia)), fdPoole); 
        
        Missatge msg;
        msgrcv(msgQueuePetitions, &msg, sizeof(Missatge) - sizeof(long), 5, 0); //ESPERAMOS TRAMA SI PLAYLIST EXISTE O NO
        if (strcmp(msg.header, "PLAY_EXIST") == 0) {
            printF("Download started!\n");
            char *playlistDirectory = malloc(strlen(clienteName) + strlen(nombreArchivoCopia) + 2); 
            sprintf(playlistDirectory, "%s/%s", clienteName, nombreArchivoCopia);   
            createDirectory(playlistDirectory);
            freeString(&playlistDirectory);

            int numSongs = numSongsDePlaylist(infoPlaylists, nombreArchivoCopia);
            descargas = realloc(descargas, (numDescargas + numSongs) * sizeof(Descarga));
            for (int i = 0; i < numSongs; i++) {
                threadDownloadSong(nombreArchivoCopia, numDescargas + i);
            }
            freeString(&nombreArchivoCopia);
            numDescargas += numSongs;
        } else if (strcmp(msg.header, "PLAY_NOEXIST") == 0) {
            printF("This playlist does not exist.\n");
        }
    } else {
        asprintf(&msgAuxiliar, "Error al descargar %s. Intentalo cuando finalizen las descargas actuales. Y realiza un 'Clear downloads'\n", nombreArchivoCopia);
        printF(msgAuxiliar);
        freeString(&msgAuxiliar);
    }
}

/*
@Finalitat: Mostra el percentatge actual de les descarregues en curs
@Paràmetres: Descarga* descargas: array de les descarregues de Bowman
@Retorn: ---
*/
void showDownloadStatus(Descarga *descargas) {
    for (int i = 0; i < numDescargas; i++) {
        if (descargas[i].nombreCancion != NULL) {
            // Descarga no eliminada
            if (descargas[i].nombrePlaylist == NULL) {
                //Es una cancion
                printF(descargas[i].nombreCancion);
                printF("\n");
            } else {
                // Es una cancion de una playlist
                asprintf(&msgAuxiliar, "%s - %s\n", descargas[i].nombrePlaylist, descargas[i].nombreCancion);
                printF(msgAuxiliar);
                freeString(&msgAuxiliar);
            }
            int numeroEqualChar = (int)(descargas[i].porcentaje / 5.0); // Asumimos que un 100% equivale a 20('=').
   
            char *cadena = malloc(numeroEqualChar + 1);
            int j = 0;
            for (j = 0; j < numeroEqualChar; j++) {
                cadena[j] = '=';
            }
            cadena[j] = '\0';
            asprintf(&msgAuxiliar, "\t%.2f%% |%s%%|\n", descargas[i].porcentaje, cadena);
            printF(msgAuxiliar);
            freeString(&msgAuxiliar);
            freeString(&cadena);
        }
    }
}

/*
@Finalitat: Mostra el percentatge actual de les descarregues en curs
@Paràmetres: char* input: comanda de l'usuari original, char* upperInput: comanda de l'usuari en majúscules
@Retorn: ---
*/
void logicaCommands(char* input, char* upperInput) {
    if (!bowmanConnected) {
        if (strcmp(upperInput, "CONNECT") == 0) {
            establishDiscoveryConnection();
            establishPooleConnection();
        } else {
            printF("You must establish a connection with the server before making any request\n");
        }
    } else {    //TRANSMISIONES DISCOVERY->BOWMAN
        if (strcmp(upperInput, "LOGOUT") == 0) {
            sig_func();
        } else if (strcmp(upperInput, "LIST SONGS") == 0) {
            requestListSongs();
        } else if (strcmp(upperInput, "LIST PLAYLISTS") == 0) {
            requestListPlaylists();
        } else if (strcmp(upperInput, "CHECK DOWNLOADS") == 0) {
            showDownloadStatus(descargas);
        } else if (strcmp(upperInput, "CLEAR DOWNLOADS") == 0) {
            cleanThreadsBowman(&descargas, &numDescargas, &maxDesc);
            showDownloadStatus(descargas);
        } else if (strstr(upperInput, "DOWNLOAD") != NULL) { 
            int numSpaces = checkDownloadCommand(upperInput);
            if (numSpaces == 1) {
                int typeFile = songOrPlaylist(upperInput);
                char *nombreArchivoCopia = NULL;
                char *nombreArchivo = strchr(input, ' ');
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
            clienteNameAux = read_until(fd, '\n');
            clienteName = verifyClientName(clienteNameAux);
            freeString(&clienteNameAux);

            pathClienteFile = read_until(fd, '\n');
            ip = read_until(fd, '\n');
            puerto = read_until(fd, '\n');
            close(fd);
            asprintf(&msgAuxiliar, "\n%s user initialized\n", clienteName);
            printF(msgAuxiliar);
            freeString(&msgAuxiliar);

            createDirectory(clienteName);
            printInfoFileBowman();
            creacionMsgQueues(&msgQueueDescargas, &msgQueuePetitions);

            char* input  = NULL;
            char* upperInput = NULL;
            while (1) { 
                printF("$ ");
                input = read_until(0, '\n'); 
                upperInput = to_upper(input);
                removeExtraSpaces(upperInput);

                logicaCommands(input, upperInput);
                freeString(&input);
                freeString(&upperInput);
            }
        }
    }
    return 0;
}