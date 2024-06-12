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
    pthread_t thread;	
    char *nombreDescargaComando;
    int fd_bowman; 
} DescargaPoole;

typedef struct {
	pthread_t thread;	
	char* user_name;
    int fd;
    DescargaPoole *descargas;
    int numDescargas; // Num total de descargas por parte de un Bowman
} ThreadPoole;

int fdPooleServer;
int fdPooleClient;
struct sockaddr_in discovery_addr;
struct sockaddr_in poole_addr;

char *msgAuxiliar;
ThreadPoole *threads;
int threads_array_size;

int fdPipe[2];
semaphore semStats;
int monolit;
pthread_mutex_t mutexDescargas;

void sig_func();
/*
@Finalitat: Inicialitzar les variables a NULL o al valor inicial desitjat.
@Paràmetres: ---
@Retorn: ---
*/
void inicializarDataPoole() {
    initScreen();
    serverName = NULL;
	pathServerFile = NULL;
    ipDiscovery = NULL; 
    puertoDiscovery = NULL;
    ipServer = NULL; 
    puertoServer = NULL;
    msgAuxiliar = NULL;
    threads = NULL;
    threads_array_size = 0;
    pthread_mutex_init(&mutexDescargas, NULL);
    SEM_constructor (&semStats);
    SEM_init (&semStats, 1); 
}

/*
@Finalitat: Obre socket amb Discovery
@Paràmetres: ---
@Retorn: ---
*/
void openDiscoverySocket() {
    fdPooleClient = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (fdPooleClient < 0) {
        perror ("Error al crear el socket de Discovery per notificar logout bowman");
        close(fdPooleClient);
        sig_func();
    }

    bzero (&discovery_addr, sizeof (discovery_addr));
    discovery_addr.sin_family = AF_INET;
    discovery_addr.sin_port = htons(atoi(puertoDiscovery)); 
    discovery_addr.sin_addr.s_addr = inet_addr(ipDiscovery);
    if (connect(fdPooleClient, (struct sockaddr*)&discovery_addr, sizeof(discovery_addr)) < 0) {
        perror("Error al conectar a Discovery per notificar logout bowman");
        close(fdPooleClient);
        sig_func();
    }
}

/*
@Finalitat: Notifica a Discovery que Poole mateix vol desconnectar-se i gestiona la resposta de Discovery.
@Paràmetres: ---
@Retorn: ---
*/
void notifyPooleDisconnected() {
    openDiscoverySocket();
    setTramaString(TramaCreate(0x06, "POOLE_DISCONNECT", serverName, strlen(serverName)), fdPooleClient);
    TramaExtended tramaExtended = readTrama(fdPooleClient);
    
    if (strcmp(tramaExtended.trama.header, "CONOK") == 0) {
        printF("Poole disconnected from Discovery succesfully\n");
    } else if (strcmp(tramaExtended.trama.header, "CONKO") == 0) {
        printF("Sorry, couldn't disconnect from Discovery\n");
    }
    freeTrama(&(tramaExtended.trama));
    close(fdPooleClient);
}

/*
@Finalitat: cancelar i matar els threads de Poole que ja hagin acabat les descarregues
@Paràmetres: ThreadPoole** thread: array de descarrgues de Poole; int numThreads: mida de l'array de threads de Poole (connexions de Bowman)
@Retorn: ---
*/
void cleanThreadsPoole(ThreadPoole** threads, int numThreads) {
    write(1, "\nYou are waiting for the completion of the sending of the songs in progress to exit the program.\n", strlen("\nYou are waiting for the completion of the sending of the songs in progress to exit the program.\n"));
    
    for (int i = 0; i < numThreads; i++) {
        if ((*threads)[i].user_name != NULL) { 
            for (int j = 0; j < (*threads)[i].numDescargas; j++) {
                pthread_join((*threads)[i].descargas[j].thread, NULL);
                pthread_cancel((*threads)[i].descargas[j].thread);
                close((*threads)[i].descargas[j].fd_bowman); 
                free((*threads)[i].descargas[j].nombreDescargaComando);
                (*threads)[i].descargas[j].nombreDescargaComando = NULL;
            }
            pthread_cancel((*threads)[i].thread);
            pthread_join((*threads)[i].thread, NULL);
            close((*threads)[i].fd); 
            free((*threads)[i].user_name);
            (*threads)[i].user_name = NULL;
        }                
    }
    free(*threads);
}

/*
@Finalitat: cancelar i matar els threads de Poole que ja hagin acabat les descarregues al igual que el thread de connexió amb aquell Bowman
@Paràmetres: ThreadPoole* thread: array de descarrgues de Poole
@Retorn: ---
*/
void cleanThreadPoole(ThreadPoole *thread) { 
    for (int j = 0; j < (*thread).numDescargas; j++) {
        pthread_cancel((*thread).descargas[j].thread);
        pthread_join((*thread).descargas[j].thread, NULL);
        close((*thread).descargas[j].fd_bowman); 
        free((*thread).descargas[j].nombreDescargaComando);
        (*thread).descargas[j].nombreDescargaComando = NULL;
    }
    pthread_cancel((*thread).thread);
    pthread_join((*thread).thread, NULL);
    close((*thread).fd); 
    free(thread->user_name);
    thread->user_name = NULL;   
}

/*
@Finalitat: Gestiona la recepció de la signal (SIGINT) i allibera els recursos utilizats fins aleshores.
@Paràmetres: ---
@Retorn: ---
*/
void sig_func() {
    cleanThreadsPoole(&threads, threads_array_size); 
    pthread_mutex_destroy(&mutexDescargas);
    SEM_destructor(&semStats);
    kill(monolit, SIGKILL);

    if (fdPipe[0] != -1) {
        close(fdPipe[0]);
    }
    if (fdPipe[1] != -1) {
        close(fdPipe[1]);
    }
    notifyPooleDisconnected();

    if (serverName != NULL) {
        freeString(&serverName);
    }
    if (pathServerFile != NULL) {
        freeString(&pathServerFile);
    }
    if (ipDiscovery != NULL) {
        freeString(&ipDiscovery);
    }
    if (puertoDiscovery != NULL) {
        freeString(&puertoDiscovery);
    }
    if (ipServer != NULL) {
        freeString(&ipServer);
    }
    if (puertoServer != NULL) {
        freeString(&puertoServer);
    }
    if (msgAuxiliar != NULL) {
        freeString(&msgAuxiliar);
    }
    close(fdPooleServer);
    close(fdPooleClient);
    destroyMutexScreen();
    exit(EXIT_SUCCESS);
}

/*
@Finalitat: Notifica a Discovery de un dels seus Bowman's ha realitzat la seva desconnexió i gestiona la resposta de Discovery.
@Paràmetres: int fd_bowman: file descriptor del Bowman a qui se li enviarà la notificació de que Discovery i Poole han gestionat bé la seva desconnexió.
@Retorn: ---
*/
void notifyBowmanLogout(int fd_bowman) {
    openDiscoverySocket();
    setTramaString(TramaCreate(0x06, "BOWMAN_LOGOUT", serverName, strlen(serverName)), fdPooleClient);
    TramaExtended tramaExtended = readTrama(fdPooleClient);
    
    if (strcmp(tramaExtended.trama.header, "CONOK") == 0) {
        setTramaString(TramaCreate(0x06, "CONOK", serverName, strlen(serverName)), fd_bowman);
    } else if (strcmp(tramaExtended.trama.header, "CONKO") == 0) {
        setTramaString(TramaCreate(0x06, "CONKO", serverName, strlen(serverName)), fd_bowman);
    }
    freeTrama(&(tramaExtended.trama));
    close(fdPooleClient);
}

/*
@Finalitat: Envia les trames de les llistes tant de cançons com de playlists
@Paràmetres: int fd_bowman: file descriptor del Bowman a qui se li enviaran les trames; char *cadena: dades pel camp data de la trama; char *header: header de la trama; size_t numMaxChars: nombre màxim de caràcters que pot contenir la trama.
@Retorn: ---
*/
void enviarTramas(int fd_bowman, char *cadena, char* header, size_t numMaxChars) {
    int i = 0;
    char *trama = NULL;
    size_t sizeData = strlen(cadena);

    // Primero de todo enviamos una trama con el numero total de tramas que procesará Bowman.
    size_t aux = sizeData;
    while (aux > numMaxChars) {
        aux -= numMaxChars; 
        i++;
    }
    char *numTramas = convertIntToString(i + 1);
    setTramaString(TramaCreate(0x02, header, numTramas, strlen(numTramas)), fd_bowman);
    freeString(&numTramas);

    if (sizeData < numMaxChars) { 
        trama = readNumChars(cadena, 0, sizeData);
        setTramaString(TramaCreate(0x02, header, trama, strlen(trama)), fd_bowman);
    } else {
        i = 0;
        while (sizeData > numMaxChars) {
            trama = readNumChars(cadena, i * numMaxChars, numMaxChars);
            setTramaString(TramaCreate(0x02, header, trama, strlen(trama)), fd_bowman); 
            sizeData -= numMaxChars; 
            i++;
            freeString(&trama);
        }
        trama = readNumChars(cadena, i * numMaxChars, sizeData);
        setTramaString(TramaCreate(0x02, header, trama, strlen(trama)), fd_bowman);
    }
    freeString(&trama);
}

/*
@Finalitat: Envia trama de la llista de cançons
@Paràmetres: int fd_bowman: file descriptor del Bowman a qui se li enviarà la cançó
@Retorn: ---
*/
void sendListSongs(int fd_bowman) {
    char *songs = NULL; 
    int totalSongs = 0;

    listSongs(serverName, &songs, &totalSongs);
    enviarTramas(fd_bowman, songs, "SONGS_RESPONSE", 239);
    freeString(&songs);
}

/*
@Finalitat: Envia els bytes de la cançó
@Paràmetres: int fd_bowman: file descriptor del Bowman a qui se li enviarà la cançó, char* directoryPath: string amb el path de la cançó; char* song: nom de la cançó; char* id: identificador de la cançó; int fileSize: mida del fitxer
@Retorn: ---
*/
void enviarDatosSong(int fd_bowman, char *directoryPath, char *song, char *id, int fileSize) {
    int bytesLeidos = 0;
    size_t len = strlen(directoryPath) + strlen(song) + 2;
    char *path = malloc(len);
    char data[244];
    memset(&data, '\0', 244);
    snprintf(path, len, "%s/%s", directoryPath, song);

    int fd_file = open(path, O_RDONLY, 0644);
    if (fd_file == -1) {
        perror("Error al crear el archivo");
        freeString(&path);
        sig_func();
    } 
    freeString(&path);

    int longitudId = strlen(id);
    char *buffer = malloc(244 - longitudId - 1);
    int i = 0;
    for (i = 0; i < longitudId; i++) {
        data[i] = id[i];
    }
    data[i] = '&';
    i++;

    do {
        bytesLeidos = read(fd_file, buffer, 244 - longitudId - 1);
        for (int j = 0; j < bytesLeidos; j++) {
            data[i + j] = buffer[j];
        }
        setTramaString(TramaCreate(0x04, "FILE_DATA", data, bytesLeidos + longitudId + 1), fd_bowman);
        usleep(20000); 
        fileSize -= bytesLeidos; 
    } while(fileSize >= 244 - longitudId - 1);

    if (fileSize > 0) {
        bytesLeidos = read(fd_file, buffer, fileSize);
        for (int j = 0; j < bytesLeidos; j++) {
            data[i + j] = buffer[j];
        }
        setTramaString(TramaCreate(0x04, "FILE_DATA", data, bytesLeidos + longitudId + 1), fd_bowman); 
    }
    freeString(&buffer);
    close(fd_file);
}

/*
@Finalitat: Busca si la cançó existeix, calcula el seu md5sum, envia trama inicial NEW_FILE i dona pas a l'enviament de les dades de la cançó
@Paràmetres: char* song: string amb el path de la cançó; int fd_bowman: file descriptor del Bowman a qui se li enviarà la cançó
@Retorn: ---
*/
void sendSong(char *song, int fd_bowman) { 
    int fileSize = 0;
    size_t len = strlen(serverName) + strlen(song) + 2;
    char *pathSong = malloc(strlen(serverName) + strlen(song) + 2);
    snprintf(pathSong, len, "%s/%s", serverName, song);

    if (searchSong(pathSong, &fileSize)) {
        setTramaString(TramaCreate(0x01, "FILE_EXIST", "", 0), fd_bowman); 

        pthread_mutex_lock(&mutexDescargas);
        char *md5sum = resultMd5sumComand(pathSong); 
        pthread_mutex_unlock(&mutexDescargas);
        freeString(&pathSong);

        if (md5sum != NULL) {
            int randomID = getRandomID();
            char *data = NULL;
            if (strchr(song, '/') != NULL) { //es cancion de una playlist 
                data = createString4Params(strchr(song, '/') + 1, convertIntToString(fileSize), md5sum, convertIntToString(randomID));
            } else { //es cancion normal
                data = createString4Params(song, convertIntToString(fileSize), md5sum, convertIntToString(randomID));
            }
            setTramaString(TramaCreate(0x04, "NEW_FILE", data, strlen(data)), fd_bowman);
            freeString(&md5sum);
            freeString(&data);

            enviarDatosSong(fd_bowman, serverName, song, convertIntToString(randomID), fileSize);
        }
    } else {
        setTramaString(TramaCreate(0x01, "FILE_NOEXIST", "", 0), fd_bowman);
    }
}

/*
@Finalitat: Funció de thread de l'enviament/descarrega de cançons
@Paràmetres: void* thread: estructura amb informació necessaria pel thread
@Retorn: ---
*/
static void *thread_function_send_song(void* thread) {
    DescargaPoole *mythread = (DescargaPoole*) thread;
    sendSong(mythread->nombreDescargaComando, mythread->fd_bowman);
    return NULL;
}

/*
@Finalitat: crea thread i allotja memòria per a cada cançó que s'ha demanat descarregar (sigui d'una playlist o no)
@Paràmetres: char* song: nom de la cançó; ThreadPoole* thread: estructura amb informació necessaria pel thread, int index: index de la posició d'aquella descarrega dins l'array de descarregues de Poole
@Retorn: ---
*/
void threadSendSong(char *song, ThreadPoole *thread, int index) { 
    thread->descargas[index].nombreDescargaComando = strdup(song);
    thread->descargas[index].fd_bowman = thread->fd; 
    if (pthread_create(&thread->descargas[index].thread, NULL, thread_function_send_song, (void *)&thread->descargas[index]) != 0) {
        perror("Error al crear el thread para la descarga");
        thread->numDescargas--;
    }
}

/*
@Finalitat: Envia la llista de les playlists que Poole conté
@Paràmetres: int fd_bowman: file descriptor per on enviarà les trames
@Retorn: ---
*/
void sendListPlaylists(int fd_bowman) {
    char *playlists = NULL;
    int totalSongs = 0;
    listPlaylists(serverName, &playlists, &totalSongs);

    char *cantidadCanciones = convertIntToString(totalSongs);
    setTramaString(TramaCreate(0x02, "PLAYLISTS_RESPONSE", cantidadCanciones, strlen(cantidadCanciones)), fd_bowman);
    freeString(&cantidadCanciones);

    enviarTramas(fd_bowman, playlists, "PLAYLISTS_RESPONSE", 234);
    freeString(&playlists);
}

/*
@Finalitat: Accedeix al path i es mou cançó a cançó
@Paràmetres: const char* path: string amb el path a la playlist demanada; ThreadPoole* thread: estructura amb informació necessaria pel thread
@Retorn: ---
*/
void accedePlaylists(const char *path, ThreadPoole *thread) { 
    char* msg = NULL;
    struct dirent *entry;
    DIR *dir = opendir(path);
    if (dir == NULL) {
        perror("Error al abrir el directorio");
        return;
    }

    int numArchivos = contarArchivosRegulares(path);
    if (numArchivos == -1) {
        return;
    }
    int i = 0;
    thread->descargas = realloc(thread->descargas, sizeof(DescargaPoole) * (thread->numDescargas + numArchivos)); 

    char *aux = strchr(path, '/') + 1;
    asprintf(&msg, "\nNew request - %s wants to download the playlist %s\nSending %s to %s. A total of %d will be sent.\n", thread->user_name, aux, aux, thread->user_name, numArchivos);
    printF(msg);
    free(msg);
    msg = NULL;
    
    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_type == DT_REG && strcmp(entry->d_name, ".DS_Store") != 0) { // Es un fichero y no es ".DS_Store"
            size_t fileNameLen = strlen(entry->d_name);
            char *subPath = malloc(strlen(path) + fileNameLen + 2); 
            sprintf(subPath, "%s/%s", path, entry->d_name);          
            
            char *subPathAux = strchr(subPath, '/') + 1;
            threadSendSong(subPathAux, thread, thread->numDescargas + i);  
            free(subPath);
            subPath = NULL;
            i++;
        }
    }
    thread->numDescargas += numArchivos;
    closedir(dir);
}
/*
@Finalitat: Envia trames si playlist demanada existeix o no
@Paràmetres: char* pathPlaylist: string amb el path a la playlist demanada; ThreadPoole* thread: estructura amb informació necessaria pel thread
@Retorn: ---
*/
void sendPlaylist(char *pathPlaylist, ThreadPoole *thread) { 
    if (searchPlaylist(pathPlaylist)) {
        setTramaString(TramaCreate(0x01, "PLAY_EXIST", "", 0), thread->fd); 
        accedePlaylists(pathPlaylist, thread); 
    } else {
        setTramaString(TramaCreate(0x01, "PLAY_NOEXIST", "", 0), thread->fd);
    }
}

/*
@Finalitat: Gestionar la nova connexió d'un Bowman i implementar la lògica principal de peticions que li arriben de Bowman
@Paràmetres: ThreadPoole* mythread: estructura amb informació necessaria pel thread
@Retorn: ---
*/
void conexionBowman(ThreadPoole* mythread) {
    char *aux;
    TramaExtended tramaExtended = readTrama(mythread->fd);
    mythread->descargas = NULL;
    aux = read_until_string(tramaExtended.trama.data, '~');
    mythread->user_name = strdup(aux);
    freeString(&aux);
    mythread->numDescargas = 0;

    asprintf(&msgAuxiliar,"\nNew user connected: %s.\n", mythread->user_name);
    printF(msgAuxiliar);
    freeString(&msgAuxiliar);

    // Transmisión Poole->Bowman para informar del estado de la conexion.
    if (strcmp(tramaExtended.trama.header, "NEW_BOWMAN") == 0) {
        setTramaString(TramaCreate(0x01, "CON_OK", "", 0), mythread->fd);
    } else {
        setTramaString(TramaCreate(0x01, "CON_KO", "", 0), mythread->fd);
        close(mythread->fd);
        freeTrama(&(tramaExtended.trama));
        cleanThreadPoole(mythread); 
    }
    freeTrama(&(tramaExtended.trama));

    //TRANSMISIONES POOLE-->BOWMAN
    while(1) {
        TramaExtended tramaExtended = readTrama(mythread->fd);
        if (strcmp(tramaExtended.trama.header, "EXIT") == 0) {    
            notifyBowmanLogout(mythread->fd);
            
            asprintf(&msgAuxiliar,"\nNew request - %s logged out\n", mythread->user_name);
            printF(msgAuxiliar);
            freeString(&msgAuxiliar);
            freeTrama(&(tramaExtended.trama));
            freeString(&mythread->user_name);
            cleanThreadPoole(mythread); 
        } else if (strcmp(tramaExtended.trama.header, "LIST_SONGS") == 0) {
            asprintf(&msgAuxiliar,"\nNew request - %s requires the list of songs.\nSending song list to %s\n", mythread->user_name, mythread->user_name);
            printF(msgAuxiliar);
            freeString(&msgAuxiliar);

            sendListSongs(mythread->fd);
        } else if (strcmp(tramaExtended.trama.header, "LIST_PLAYLISTS") == 0) {
            asprintf(&msgAuxiliar,"\nNew request - %s requires the list of playlists.\nSending playlist list to %s\n", mythread->user_name, mythread->user_name);
            printF(msgAuxiliar);
            freeString(&msgAuxiliar);

            sendListPlaylists(mythread->fd);
        } else if (strcmp(tramaExtended.trama.header, "CHECK DOWNLOADS") == 0) {
            printF("You have no ongoing or finished downloads\n");
        } else if (strcmp(tramaExtended.trama.header, "CLEAR DOWNLOADS") == 0) {
            printF("No downloads to clear available\n");
        } else if (strstr(tramaExtended.trama.header, "DOWNLOAD") != NULL) {  
            cleanPadding(tramaExtended.trama.data, '~');
            char *upperInput = to_upper(tramaExtended.trama.data);
            removeExtraSpaces(upperInput);

            int typeFile = songOrPlaylist(upperInput);
            freeString(&upperInput);
            if (typeFile == 1) {
                mythread->descargas = realloc(mythread->descargas, sizeof(DescargaPoole) * (mythread->numDescargas + 1)); 
                aux = strdup(tramaExtended.trama.data);
                asprintf(&msgAuxiliar, "\nNew request - %s wants to download %s\nSending %s to %s\n", mythread->user_name, tramaExtended.trama.data, tramaExtended.trama.data, mythread->user_name);
                printF(msgAuxiliar);
                freeString(&msgAuxiliar);

                threadSendSong(aux, mythread, mythread->numDescargas);
                freeString(&aux);
                mythread->numDescargas++;
            } else {
                size_t len = strlen(serverName) + strlen(tramaExtended.trama.data) + 2;
                char *aux = (char *)malloc(len);
                snprintf(aux, len, "%s/%s", serverName, tramaExtended.trama.data);
        
                sendPlaylist(aux, mythread); 
                freeString(&aux);
            }
        } else if (strcmp(tramaExtended.trama.header, "CHECK_OK") == 0) {
            char *auxData = strdup(tramaExtended.trama.data);
            cleanPadding(auxData, '~');
            asprintf(&msgAuxiliar, "\nSong %s sent and downloaded successfully!\n", auxData);
            printF(msgAuxiliar);
            freeString(&msgAuxiliar);
            freeString(&auxData);

            // Mandamos el nombre de la cancion por la Pipe para que lo reciba el Monolit.
            SEM_wait(&semStats);
            write(fdPipe[1], tramaExtended.trama.data, 256 - 3 - 8); 
        } else if (strcmp(tramaExtended.trama.header, "CHECK_KO") == 0) {
            printF("The download of the song was unsuccessfull, try again\n");
        } else {
            printF("Unknown command\n");
        }
        freeTrama(&tramaExtended.trama);  
    }
    freeTrama(&(tramaExtended.trama));  
    freeString(&mythread->user_name);
}

/*
@Finalitat: Implementació de la funció de thread de cada Bowman
@Paràmetres: void* thread: estructura amb informació pel thread
@Retorn: void*
*/
static void *thread_function_bowman(void* thread) {
    ThreadPoole *mythread = (ThreadPoole*) thread;
    conexionBowman(mythread);
    return NULL;
}

/*
@Finalitat: Protocol d'acceptació de conexions de Bowman 
@Paràmetres: ---
@Retorn: ---
*/
void connect_Bowman() {
    socklen_t bAddr = sizeof(poole_addr);
    int fd_bowman = accept(fdPooleServer, (struct sockaddr *)&poole_addr, &bAddr);
    if (fd_bowman < 0) {
        perror("Error al aceptar la conexión de Bowman");
        close(fd_bowman);
        return;
    }
    threads = realloc(threads, sizeof(ThreadPoole) * (threads_array_size + 1)); 
    threads[threads_array_size].fd = fd_bowman;
    
    if (pthread_create(&threads[threads_array_size].thread, NULL, thread_function_bowman, (void *)&threads[threads_array_size]) != 0) {
        perror("Error al crear el thread inicial para Bowman");
    }
    threads_array_size++;
}

/*
@Finalitat: Obrir socket per a conexions de Bowmans
@Paràmetres: ---
@Retorn: ---
*/
void waitingForRequests() {
    fdPooleServer = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (fdPooleServer < 0) {
        perror ("Error al crear el socket de Bowman");
        close(fdPooleServer);
        sig_func();
    }
    bzero (&poole_addr, sizeof (poole_addr));
    poole_addr.sin_family = AF_INET;
    poole_addr.sin_port = htons(atoi(puertoServer)); 
    poole_addr.sin_addr.s_addr = inet_addr(ipServer);

    if (bind(fdPooleServer, (struct sockaddr*)&poole_addr, sizeof(poole_addr)) < 0) {
        perror("Error al enlazar el socket de Poole");
        close(fdPooleServer);
        sig_func();
    }
    listen(fdPooleServer, 20); 
    printF("Waiting for connections...\n");
    while(1) {
        connect_Bowman();
    }    
}

/*
@Finalitat: Obrir socket amb Discovery i enviar/rebre les trames pertinents
@Paràmetres: ---
@Retorn: ---
*/
void establishDiscoveryConnection() {
    fdPooleClient = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (fdPooleClient < 0) {
        perror ("Error al crear el socket de Discovery");
        close(fdPooleClient);
        sig_func();
    }
    bzero (&discovery_addr, sizeof (discovery_addr));
    discovery_addr.sin_family = AF_INET;
    discovery_addr.sin_port = htons(atoi(puertoDiscovery)); 
    discovery_addr.sin_addr.s_addr = inet_addr(ipDiscovery);

    if (connect(fdPooleClient, (struct sockaddr*)&discovery_addr, sizeof(discovery_addr)) < 0) {
        perror("Error al conectar a Discovery");
        close(fdPooleClient);
        sig_func();
    }
    //TRANSMISIONES POOLE->DISCOVERY
    char* aux = NULL;
    aux = createString3Params(serverName, ipServer, puertoServer);
    setTramaString(TramaCreate(0x01, "NEW_POOLE", aux, strlen(aux)), fdPooleClient);
    freeString(&aux);

    TramaExtended tramaExtended = readTrama(fdPooleClient);    
    if (strcmp(tramaExtended.trama.header,"CON_OK") == 0)  {
        close(fdPooleClient);
        freeTrama(&(tramaExtended.trama));
        waitingForRequests();
    } else if (strcmp(tramaExtended.trama.header,"CON_KO") == 0) {
        printF("Couldn't connect to Discovery server, try again\n");
    }
    freeTrama(&(tramaExtended.trama));
    close(fdPooleClient);
}

/*
@Finalitat: Escriure al fitxer stats.txt
@Paràmetres: ---
@Retorn: ---
*/
void funcionMonolit() {
    while(1) {
        char *descargaCancion = read_until(fdPipe[0], '~');
        size_t length = 256 - strlen(descargaCancion) - 3 - 8;
        char *basura = malloc(length);
        read(fdPipe[0], basura, length);
        freeString(&basura);

        size_t len = strlen(serverName) + strlen("stats.txt") + 2; 
        char *path = malloc(len);
        snprintf(path, len, "%s/%s", serverName, "stats.txt");

        int fd_file = open(path, O_RDWR, 0644);
        freeString(&path);
        if (fd_file == -1) {
            perror("Error al crear el archivo");
            sig_func();
        }

        int flag = 0;
        while(1) {
            char *cancion = read_until(fd_file, '\n');
            if (cancion == NULL) {
                break; //EOF
            }
            if (strcmp(descargaCancion, cancion) == 0) {
                flag = 1;
                int numDescargas = atoi(read_until(fd_file, '\n'));
                numDescargas++;
                char *numD = convertIntToString(numDescargas);
                int offsetBack = -(strlen(numD) + 1); 
                
                lseek(fd_file, ((offsetBack)*sizeof(char)), SEEK_CUR);
                write(fd_file, numD, strlen(numD));
                write(fd_file, "\n", 1);

                lseek(fd_file, 0, SEEK_SET);

                freeString(&numD);
                freeString(&cancion);
                break;
            }
            freeString(&cancion);
        }
        // En caso de que la cancion no este registrada en el fichero.
        if (!flag) {
            lseek(fd_file, 0, SEEK_END);
            write(fd_file, descargaCancion, strlen(descargaCancion));
            write(fd_file, "\n", 1);
            write(fd_file, "1", 1);
            write(fd_file, "\n", 1);

            lseek(fd_file, 0, SEEK_SET);
        }
        freeString(&descargaCancion);
        close(fd_file);
        SEM_signal(&semStats);
    }
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
            serverName = read_until(fd, '\n');
            pathServerFile = read_until(fd, '\n');
            ipDiscovery = read_until(fd, '\n');
            puertoDiscovery = read_until(fd, '\n');
            ipServer = read_until(fd, '\n');
            puertoServer = read_until(fd, '\n');            
            createDirectory(serverName);
            createStatsFile(serverName);

            if (pipe(fdPipe) == -1) {
                perror("Crecion Pipe sin exito");
            }
            monolit = fork();
            if (monolit > 0) {
                printInfoFile();
                close(fd);
                close(fdPipe[0]); //Lectura Cerrada
                fdPipe[0] = -1;
                establishDiscoveryConnection();
            } else {
                signal(SIGINT, SIG_IGN);
                close(fdPipe[1]); // Escritura Cerrada
                fdPipe[1] = -1;
                funcionMonolit();
            }
        }
    }
    return 0;
}