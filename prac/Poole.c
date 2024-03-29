/*
Autores:
    Alba Mayol Lozano -->alba.mayol
    Kevin Eljarrat Ohayon --> kevin.eljarrat
    LaSalle - Sistemes Operatius
*/

#include "Trama.h"

dataPoole dPoole;

void sig_func();
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
    dPoole.threads = NULL;
    dPoole.threads_array_size = 0;
}

void openDiscoverySocket() {
    dPoole.fdPooleClient = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (dPoole.fdPooleClient < 0) {
        perror ("Error al crear el socket de Discovery per notificar logout bowman");
        close(dPoole.fdPooleClient);
        sig_func();
    }

    bzero (&dPoole.discovery_addr, sizeof (dPoole.discovery_addr));
    dPoole.discovery_addr.sin_family = AF_INET;
    dPoole.discovery_addr.sin_port = htons(atoi(dPoole.puertoDiscovery)); 
    dPoole.discovery_addr.sin_addr.s_addr = inet_addr(dPoole.ipDiscovery);

    if (connect(dPoole.fdPooleClient, (struct sockaddr*)&dPoole.discovery_addr, sizeof(dPoole.discovery_addr)) < 0) {
        perror("Error al conectar a Discovery per notificar logout bowman");
        close(dPoole.fdPooleClient);
        sig_func();
    }
}

void notifyPooleDisconnected() {
    openDiscoverySocket();
    setTramaString(TramaCreate(0x06, "POOLE_DISCONNECT", dPoole.serverName, strlen(dPoole.serverName)), dPoole.fdPooleClient);
    TramaExtended tramaExtended = readTrama(dPoole.fdPooleClient);
    
    if (strcmp(tramaExtended.trama.header, "CONOK") == 0) {
        printF("Poole disconnected from Discovery succesfully\n");
    } else if (strcmp(tramaExtended.trama.header, "CONKO") == 0) {
        printF("Sorry, couldn't disconnect from Discovery\n");
    }

    freeTrama(&(tramaExtended.trama));
    close(dPoole.fdPooleClient);
}

/*
@Finalitat: Manejar la recepción de la signal (SIGINT) y liberar los recursos utilizados hasta el momento.
@Paràmetres: ---
@Retorn: ---
*/
void sig_func() {
    close(dPoole.fdPooleServer);
    close(dPoole.fdPooleClient);

    if (dPoole.fdPipe[0] != -1) {
        close(dPoole.fdPipe[0]);
    }

    if (dPoole.fdPipe[1] != -1) {
        close(dPoole.fdPipe[1]);
    }

    pthread_mutex_destroy(&dPoole.mutexStats);

    notifyPooleDisconnected();
    
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
    cleanThreadsPoole(&dPoole.threads, dPoole.threads_array_size); 

    exit(EXIT_FAILURE);
}

/*
@Finalitat: Printa la información leída de configPoole
@Paràmetres: ---
@Retorn: ---
*/
void printInfoFile() {
    asprintf(&dPoole.msg, "\nFile read correctly:\nServer - %s\nServer Directory - %s\nIP Discovery - %s\nPort Server - %s\nIP Server - %s\nPort Server - %s\n\n", dPoole.serverName, dPoole.pathServerFile, dPoole.ipDiscovery, dPoole.puertoDiscovery, dPoole.ipServer, dPoole.puertoServer);
    printF(dPoole.msg);
    freeString(&dPoole.msg);
}

void notifyBowmanLogout(int fd_bowman) {
    openDiscoverySocket();
    setTramaString(TramaCreate(0x06, "BOWMAN_LOGOUT", dPoole.serverName, strlen(dPoole.serverName)), dPoole.fdPooleClient);
    TramaExtended tramaExtended = readTrama(dPoole.fdPooleClient);
    
    if (strcmp(tramaExtended.trama.header, "CONOK") == 0) {
        setTramaString(TramaCreate(0x06, "CONOK", dPoole.serverName, strlen(dPoole.serverName)), fd_bowman);
    } else if (strcmp(tramaExtended.trama.header, "CONKO") == 0) {
        setTramaString(TramaCreate(0x06, "CONKO", dPoole.serverName, strlen(dPoole.serverName)), fd_bowman);
    }

    freeTrama(&(tramaExtended.trama));
    close(dPoole.fdPooleClient);
}

void listSongs(const char *path, char **fileNames, int *totalSongs) {
    struct dirent *entry;
    DIR *dir = opendir(path);

    if (dir == NULL) {
        perror("Error al abrir el directorio");
        return;
    }

    size_t totalLength = 0;
    int isFirstFile = 1; 

    *fileNames = malloc(1);
    (*fileNames)[0] = '\0'; 

    while ((entry = readdir(dir)) != NULL) {
        if ((strstr(entry->d_name, ".mp3") != NULL) && (strcmp(entry->d_name, ".DS_Store") != 0)) {
            size_t fileNameLen = strlen(entry->d_name);
            *fileNames = realloc(*fileNames, totalLength + fileNameLen + 1); 
            
            if (*fileNames == NULL) {
                perror("Error en realloc");
                closedir(dir);
                return;
            }

            if (!isFirstFile) {
                strcat(*fileNames, "&");
            } else {
                isFirstFile = 0; 
            }

            strcat(*fileNames, entry->d_name);
            totalLength += fileNameLen + 1; 
            (*totalSongs) += 1;
        }
    }

    closedir(dir);
}

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
        // 256 - Type(1 Byte) - header_length(2 Bytes) - Header(14 Bytes) = 239 Bytes disponibles para list songs
        // 256 - Type(1 Byte) - header_length(2 Bytes) - Header(19 Bytes) = 234 Bytes disponibles para list playlists
        trama = readNumChars(cadena, 0, sizeData);
        //asprintf(&dPoole.msg,"\nTrama %d: %s.\n", i + 1, trama);
        //printF(dPoole.msg);
        //freeString(&dPoole.msg);
        setTramaString(TramaCreate(0x02, header, trama, strlen(trama)), fd_bowman);
    } else {
        i = 0;
        while (sizeData > numMaxChars) {
            trama = readNumChars(cadena, i * numMaxChars, numMaxChars);
            //asprintf(&dPoole.msg,"\nTrama %d: %s.\n", i + 1, trama);
            //printF(dPoole.msg);
            //freeString(&dPoole.msg);
            setTramaString(TramaCreate(0x02, header, trama, strlen(trama)), fd_bowman); 
            sizeData -= numMaxChars; 
            i++;
            freeString(&trama);
        }
        trama = readNumChars(cadena, i * numMaxChars, sizeData);
        //asprintf(&dPoole.msg,"\nTrama %d: %s.\n", i + 1, trama);
        //printF(dPoole.msg);
        //freeString(&dPoole.msg);
        setTramaString(TramaCreate(0x02, header, trama, strlen(trama)), fd_bowman);
    }
    
    freeString(&trama);
}

void sendListSongs(int fd_bowman) {
    char *songs = NULL; 

    int totalSongs = 0;

    listSongs(dPoole.serverName, &songs, &totalSongs);

    enviarTramas(fd_bowman, songs, "SONGS_RESPONSE", 239);

    freeString(&songs);
}

void listPlaylists(const char *path, char **fileNames, int *totalSongs) {
    struct dirent *entry;
    DIR *dir = opendir(path);

    if (dir == NULL) {
        perror("Error al abrir el directorio");
        return;
    }

    size_t totalLength = 1; 
    *fileNames = malloc(1);
    (*fileNames)[0] = '\0'; 

    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_type == DT_DIR && strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) { // Es un directorio y no es "." ni ".."
            size_t fileNameLen = strlen(entry->d_name);
            char *subPath = malloc(strlen(path) + fileNameLen + 2); 
            sprintf(subPath, "%s/%s", path, entry->d_name);

            char *subSongs = NULL;

            listSongs(subPath, &subSongs, totalSongs);

            size_t newLength = totalLength + fileNameLen + 1; // Longitud del nombre de archivo y el separador '&'

            if (subSongs != NULL) { // Si subSongs no es nulo, agrega su longitud más un carácter para el separador '&'
                newLength += strlen(subSongs) + 1;
            }

            char *temp = realloc(*fileNames, newLength);
            if (temp == NULL) {
                perror("Error al asignar memoria");
                free(subPath);
                freeString(&subSongs);
                closedir(dir);
                return;
            } else {
                *fileNames = temp;

                if (totalLength > 1) {
                    strcat(*fileNames, "#");
                }
                strcat(*fileNames, entry->d_name);

                if (subSongs != NULL && strlen(subSongs) > 0) {
                    strcat(*fileNames, "&");
                    strcat(*fileNames, subSongs);
                }
            }

            free(subPath);
            freeString(&subSongs);

            totalLength = newLength;
        }
    }

    closedir(dir);
}

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
        //usleep(60000); //valor de Malé 
        //usleep(20000); //valor de Ferran
        usleep(10000); //Valor testeado

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

int searchSong(char *pathSongPlaylist, int *fileSize) {
    struct stat st;
    int found = 0;

    if (stat(pathSongPlaylist, &st) == 0) {
        found = 1;
        *fileSize = st.st_size;
    } else {
        perror("Error al encontrar la canción/playlist\n");
        *fileSize = 0;
    }
    return found;
}

int searchPlaylist(char *pathSongPlaylist) {
    struct stat st;
    int found = 0;

    if (stat(pathSongPlaylist, &st) == 0) {
        found = 1;
    } else {
        perror("Error al encontrar la canción/playlist\n");
    }
    return found;
}

int getRandomID() { //SIEMPRE NOS GENERA LOS MISMOS ID'S
    //srand(time(NULL)); ERROR A COMENTAR EN LA MEMORIA
    return (rand() % 999) + 1; 
}

void sendSong(char *song, int fd_bowman) { //si enviamos una cancion de una playlist, añadir previamente char* song: sutton/song1.mp3
    int fileSize = 0;

    size_t len = strlen(dPoole.serverName) + strlen(song) + 2;
    char *pathSong = malloc(strlen(dPoole.serverName) + strlen(song) + 2);
    snprintf(pathSong, len, "%s/%s", dPoole.serverName, song);

    if (searchSong(pathSong, &fileSize)) {
        setTramaString(TramaCreate(0x01, "FILE_EXIST", "", 0), fd_bowman); 

        pthread_mutex_lock(&dPoole.mutexDescargas);
        char *md5sum = resultMd5sumComand(pathSong); 
        pthread_mutex_unlock(&dPoole.mutexDescargas);
        printF("mdsum original: ");
        printF(md5sum);
        printF("\n");
        freeString(&pathSong);
        if (md5sum != NULL) {
            int randomID = getRandomID();
            asprintf(&dPoole.msg,"\nRANDOM ID: %d\n", randomID);
            printF(dPoole.msg);
            freeString(&dPoole.msg);
            //char *cancion = NULL; //FASE 4
            char *data = NULL;
            if (strchr(song, '/') != NULL) { //es cancion de una playlist --> Quitamos sutton de song (sutton/song1.mp3)
                data = createString4Params(strchr(song, '/') + 1, convertIntToString(fileSize), md5sum, convertIntToString(randomID));
                //cancion = strdup(strchr(song, '/') + 1); //FASE 4
            } else { //es cancion normal
                data = createString4Params(song, convertIntToString(fileSize), md5sum, convertIntToString(randomID));
                //cancion = strdup(song); //FASE 4
            }
    
            setTramaString(TramaCreate(0x04, "NEW_FILE", data, strlen(data)), fd_bowman);
            freeString(&md5sum);
            freeString(&data);

            enviarDatosSong(fd_bowman, dPoole.serverName, song, convertIntToString(randomID), fileSize);
        }
    } else {
        setTramaString(TramaCreate(0x01, "FILE_NOEXIST", "", 0), fd_bowman);
    }
}

static void *thread_function_send_song(void* thread) {
    DescargaPoole *mythread = (DescargaPoole*) thread;
    
    sendSong(mythread->nombreDescargaComando, mythread->fd_bowman);

    // si descomentamos salta bucle unknown comand 
    //pthread_cancel(mythread->thread);
    //pthread_join(mythread->thread, NULL);

    return NULL;
}

void threadSendSong(char *song, ThreadPoole *thread, int index) { //TODO si enviamos una cancion de una playlist, añadir previamente char* song: sutton/song1.mp3
    thread->descargas[index].nombreDescargaComando = strdup(song);
    thread->descargas[index].fd_bowman = thread->fd; 

    if (pthread_create(&thread->descargas[index].thread, NULL, thread_function_send_song, (void *)&thread->descargas[index]) != 0) {
        perror("Error al crear el thread para la descarga");
        thread->numDescargas--;
    }
}

void sendListPlaylists(int fd_bowman) {
    char *playlists = NULL;
    int totalSongs = 0;

    listPlaylists(dPoole.serverName, &playlists, &totalSongs);

    char *cantidadCanciones = convertIntToString(totalSongs);
    setTramaString(TramaCreate(0x02, "PLAYLISTS_RESPONSE", cantidadCanciones, strlen(cantidadCanciones)), fd_bowman);
    freeString(&cantidadCanciones);

    enviarTramas(fd_bowman, playlists, "PLAYLISTS_RESPONSE", 234);
    freeString(&playlists);
}

int contarArchivosRegulares(const char *path) {
    int numArchivos = 0;
    struct dirent *entry;
    DIR *dir = opendir(path);

    if (dir == NULL) {
        perror("Error al abrir el directorio");
        return -1; 
    }

    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_type == DT_REG && strcmp(entry->d_name, ".DS_Store") != 0) { // Es un fichero y no es ".DS_Store"
            numArchivos++;
        }
    }

    closedir(dir);
    return numArchivos;
}

void accedePlaylists(const char *path, ThreadPoole *thread) { //path = Pepe/sutton
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
    
    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_type == DT_REG && strcmp(entry->d_name, ".DS_Store") != 0) { // Es un fichero y no es ".DS_Store"
            size_t fileNameLen = strlen(entry->d_name);
            char *subPath = malloc(strlen(path) + fileNameLen + 2); 
            sprintf(subPath, "%s/%s", path, entry->d_name);          
            
            char *subPathAux = strchr(subPath, '/') + 1;
            
            threadSendSong(subPathAux, thread, thread->numDescargas + i); //sutton/song1.mp3 
            freeString(&subPath);
            i++;
        }
    }
    thread->numDescargas += numArchivos;
    
    closedir(dir);
}

void sendPlaylist(char *pathPlaylist, ThreadPoole *thread) { //Pepe/sutton
    if (searchPlaylist(pathPlaylist)) {
        setTramaString(TramaCreate(0x01, "PLAY_EXIST", "", 0), thread->fd); 
        accedePlaylists(pathPlaylist, thread); 
    } else {
        setTramaString(TramaCreate(0x01, "PLAY_NOEXIST", "", 0), thread->fd);
    }
}

void conexionBowman(ThreadPoole* mythread) {
    TramaExtended tramaExtended = readTrama(mythread->fd);
    mythread->descargas = NULL;
    mythread->user_name = strdup(read_until_string(tramaExtended.trama.data, '~'));
    mythread->numDescargas = 0;

    asprintf(&dPoole.msg,"\nNew user connected: %s.\n", mythread->user_name);
    printF(dPoole.msg);
    freeString(&dPoole.msg);

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
            
            asprintf(&dPoole.msg,"\nNew request - %s logged out\n", mythread->user_name);
            printF(dPoole.msg);
            freeString(&dPoole.msg);

            freeTrama(&(tramaExtended.trama));
            cleanThreadPoole(mythread); 
        } else if (strcmp(tramaExtended.trama.header, "LIST_SONGS") == 0) {
            asprintf(&dPoole.msg,"\nNew request - %s requires the list of songs.\nSending song list to %s\n", mythread->user_name, mythread->user_name);
            printF(dPoole.msg);
            freeString(&dPoole.msg);

            sendListSongs(mythread->fd);
        } else if (strcmp(tramaExtended.trama.header, "LIST_PLAYLISTS") == 0) {
            asprintf(&dPoole.msg,"\nNew request - %s requires the list of playlists.\nSending playlist list to %s\n", mythread->user_name, mythread->user_name);
            printF(dPoole.msg);
            freeString(&dPoole.msg);

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
                char *aux = strdup(tramaExtended.trama.data);

                threadSendSong(aux, mythread, mythread->numDescargas);
                freeString(&aux);
                mythread->numDescargas++;
            } else {
                size_t len = strlen(dPoole.serverName) + strlen(tramaExtended.trama.data) + 2;
                char *aux = (char *)malloc(len);
                snprintf(aux, len, "%s/%s", dPoole.serverName, tramaExtended.trama.data);
        
                sendPlaylist(aux, mythread); //Pepe/sutton
            }
        } else if (strcmp(tramaExtended.trama.header, "CHECK_OK") == 0) {
            printF("Song sent and downloaded successfully!\n");
            /*printF("\n");
            printF(tramaExtended.trama.data);
            printF("\n");*/

            // Mandamos el nombre de la cancion por la Pipe para que lo reciba el Monolit.
            pthread_mutex_lock(&dPoole.mutexStats); //FASE 4
            write(dPoole.fdPipe[1], tramaExtended.trama.data, 256 - 3 - 8); //FASE 4
            //freeString(&cancion); //FASE 4
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

static void *thread_function_bowman(void* thread) {
    ThreadPoole *mythread = (ThreadPoole*) thread;
    conexionBowman(mythread);
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

    dPoole.threads = realloc(dPoole.threads, sizeof(ThreadPoole) * (dPoole.threads_array_size + 1)); 
    dPoole.threads[dPoole.threads_array_size].fd = fd_bowman;

    if (pthread_create(&dPoole.threads[dPoole.threads_array_size].thread, NULL, thread_function_bowman, (void *)&dPoole.threads[dPoole.threads_array_size]) != 0) {
        perror("Error al crear el thread inicial para Bowman");
    }
    dPoole.threads_array_size++;
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

    listen(dPoole.fdPooleServer, 20); 
    printF("Waiting for connections...\n");
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
    setTramaString(TramaCreate(0x01, "NEW_POOLE", aux, strlen(aux)), dPoole.fdPooleClient);
    freeString(&aux);

    TramaExtended tramaExtended = readTrama(dPoole.fdPooleClient);    

    if (strcmp(tramaExtended.trama.header,"CON_OK") == 0)  {
        close(dPoole.fdPooleClient);
        freeTrama(&(tramaExtended.trama));
        waitingForRequests();
    } else if (strcmp(tramaExtended.trama.header,"CON_KO") == 0) {
        //PRINT DE QUE NO SE HA PODIDO ESTABLECER LA COMUNICACION CON DISCOVERY
    }
    
    freeTrama(&(tramaExtended.trama));

    close(dPoole.fdPooleClient);
}

void funcionMonolit() {
    char *descargaCancion = read_until(dPoole.fdPipe[0], '~');
    size_t length = 256 - strlen(descargaCancion) - 3 - 8;
    char *basura = malloc(length);
    read(dPoole.fdPipe[0], basura, length);
    freeString(&basura);

    size_t len = strlen(dPoole.serverName) + strlen("stats.txt") + 2; //Pepe/stats.txt\0
    char *path = malloc(len);
    snprintf(path, len, "%s/%s", dPoole.serverName, "stats.txt");

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
            printF("VACIO");
            break; //EOF
        }
        int numDescargas = atoi(read_until(fd_file, '\n'));
        asprintf(&dPoole.msg, "Num descargas: %d\n", numDescargas);
        printF(dPoole.msg);
        freeString(&dPoole.msg);
        if (strcmp(descargaCancion, cancion) == 0) {
            flag = 1;
            numDescargas++;
            printF("ALBA");

            lseek(fd_file, -(sizeof(char)), SEEK_CUR);

            char *numD = convertIntToString(numDescargas); //"2";
            printF("\n");
            printF(numD);
            printF("\n");
            write(fd_file, &numD, strlen(numD));

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

    pthread_mutex_unlock(&dPoole.mutexStats);
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
            
            createDirectory(dPoole.serverName);

            createStatsFile(dPoole.serverName);

            if (pipe(dPoole.fdPipe) == -1) {
                perror("Crecion Pipe sin exito");
            }

            int monolit = fork();
            if (monolit > 0) {
                // El proceso padre(Poole) le avisará al hijo(Monolit) de que hay una nueva descarga.
                // Le enviará el nombre de la cancion por una pipe.
                printInfoFile();
                close(fd);
                close(dPoole.fdPipe[0]); //Lectura Cerrada
                dPoole.fdPipe[0] = -1;

                establishDiscoveryConnection();

                sig_func();
            } else {
                close(dPoole.fdPipe[1]); // Escritura Cerrada
                dPoole.fdPipe[1] = -1;
                while(1) {
                    funcionMonolit();
                }
            }
        }
    }
    return 0;
}