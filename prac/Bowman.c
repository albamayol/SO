/*
Autores:
    Alba Mayol Lozano -->alba.mayol
    Kevin Eljarrat Ohayon --> kevin.eljarrat
*/

#include "Trama.h"

dataBowman dBowman;

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
    setTramaString(TramaCreate(0x01, "NEW_BOWMAN", aux), dBowman.fdDiscovery);
    freeString(&aux);

    Trama trama = readTrama(dBowman.fdDiscovery);

    if (strcmp(trama.header,"CON_OK") == 0)  {
        separaDataToElement(trama.data, &dBowman.pooleConnected);
        asprintf(&dBowman.msg, "%s connected to HAL 9000 system, welcome music lover!\n", dBowman.clienteName);
        printF(dBowman.msg);
        freeString(&dBowman.msg);
    } else if (strcmp(trama.header,"CON_KO") == 0) {
        write(1, "CON_KO\n", strlen("CON_KO\n"));
    }

    freeTrama(&trama);
    close(dBowman.fdDiscovery);
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

    // Transmission Bowman->Poole
    setTramaString(TramaCreate(0x01, "NEW_BOWMAN", dBowman.clienteName), dBowman.fdPoole);

    // Recepción Poole->Bowman para comprobar el estado de la conexion.
    Trama trama = readTrama(dBowman.fdPoole);
    if (strcmp(trama.header, "CON_OK") == 0) {
        dBowman.bowmanConnected = 1;
    } else if (strcmp(trama.header, "CON_KO") == 0) {
        close(dBowman.fdPoole);
    }

    freeTrama(&trama);
}

void juntarTramasSongs(int numTramas, char **songs) {
    int i = 0;
    char aux[257];
    size_t totalSize = 0; 

    while(i < numTramas) {
        read(dBowman.fdPoole, aux, 256);
        aux[256] = '\0';

        size_t dataSize = strlen(aux + 17);

        *songs = realloc(*songs, totalSize + dataSize + 1);
        if (*songs == NULL) {
            break;
        }

        // Copiamos los datos de la trama actual a songs
        memcpy(*songs + totalSize, aux + 17, dataSize);
        totalSize += dataSize;

        (*songs)[totalSize] = '\0';
        i++;
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

void checkPooleConnection(ssize_t bytesLeidos) {
    // Comprobación si Poole ha cerrado conexión
    if (bytesLeidos <= 0) {
        dBowman.bowmanConnected = 0;
        close(dBowman.fdPoole);
        asprintf(&dBowman.msg, "\n¡Alert: %s disconnected because the server connection has ended!\n", dBowman.clienteName);
        printF(dBowman.msg);
        freeString(&dBowman.msg);
        sig_func();
    }
}

void requestListSongs() {
    int numCanciones = 0;
    char aux[257], *songs = NULL, **canciones = NULL;

    setTramaString(TramaCreate(0x02, "LIST_SONGS", ""), dBowman.fdPoole);

    // Lectura cantidad de tramas que recibiremos
    ssize_t bytesLeidos = read(dBowman.fdPoole, aux, 256);
    aux[256] = '\0';

    checkPooleConnection(bytesLeidos);

    int numTramas = atoi(aux + 17);
    juntarTramasSongs(numTramas, &songs);
    numCanciones = procesarTramasSongs(&canciones, songs);
    printarSongs(numCanciones, &canciones);

    free(canciones);
    freeString(&songs);
}

char *juntarTramasPlaylists(int numTramas) {
    int i = 0;
    char aux[257], *playlists = NULL;
    ssize_t totalSize = 0; 

    while(i < numTramas) {
        read(dBowman.fdPoole, aux, 256);
        aux[256] = '\0';

        int dataSize = strlen(aux + 17);

        playlists = realloc(playlists, totalSize + dataSize + 1);
        if (playlists == NULL) {
            break;
        }

        // Copiamos los datos de la trama actual a songs
        memcpy(playlists + totalSize, aux + 17, dataSize);
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

void printarPlaylists(int numListas, char ***listas, int *numCancionesPorLista) {
    asprintf(&dBowman.msg, "\nThere are %d lists available for download:", numListas);
    printF(dBowman.msg);
    freeString(&dBowman.msg);

    for (int i = 0; i < numListas; i++) {
        asprintf(&dBowman.msg, "\n%d. %s", i + 1, listas[i][0]);
        printF(dBowman.msg);
        freeString(&dBowman.msg);

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
    char aux[257], *playlists = NULL, ***listas = NULL;
    int numListas = 0;
    int *numCancionesPorLista = malloc(sizeof(int)); 
    *numCancionesPorLista = 0;

    setTramaString(TramaCreate(0x02, "LIST_PLAYLISTS", ""), dBowman.fdPoole);

    // Lectura cantidad de canciones
    ssize_t bytesLeidos = read(dBowman.fdPoole, aux, 256);
    aux[256] = '\0';

    checkPooleConnection(bytesLeidos);

    int numCanciones = atoi(aux + 17);

    // Lectura cantidad de tramas que recibiremos
    read(dBowman.fdPoole, aux, 256);
    aux[256] = '\0';

    int numTramas = atoi(aux + 17);

    playlists = juntarTramasPlaylists(numTramas);

    listas = procesarTramasPlaylists(playlists, &numCancionesPorLista, numCanciones, &numListas);

    printarPlaylists(numListas, listas, numCancionesPorLista);

    free(playlists);
}

int requestLogout() {  
    setTramaString(TramaCreate(0x06, "EXIT", dBowman.clienteName), dBowman.fdPoole);

    char aux[257];
    ssize_t bytesLeidos = read(dBowman.fdPoole, aux, 256);
    aux[256] = '\0';

    //dBowman.bowmanConnected = 0;
    if (bytesLeidos <= 0) {
        close(dBowman.fdPoole);
        asprintf(&dBowman.msg, "\n¡Alert: %s disconnected because the server connection has ended!\n", dBowman.clienteName);
        printF(dBowman.msg);
        freeString(&dBowman.msg);
        return 1;
    }

    char header[6]; //CONOK o CONKO
    int indiceInicio = 3;
    int longitudHeader = 8 - 3; 

    // Usamos strncpy para copiar la subcadena desde aux a subcadena
    strncpy(header, aux + indiceInicio, longitudHeader);
    header[longitudHeader] = '\0';

    if (strcmp(header, "CONOK") == 0) {
        printF(header);
        //OK
        close(dBowman.fdPoole);
        return 1;
    } else if (strcmp(header, "CONKO")) {
        printF(header);
        //KO
        printF("Sorry, couldn't logout, try again\n");
        return 0;
    }
    return 2;
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


void getIdData(char* buffer, int *id, char** dataFile) {    //separamos id & datos archivo
    int counter = 0, i = 0, lengthData = 244; //256 - (1 - 2 - 9 (header: "FILE_DATA")) = 244
    char* idString = (char *) malloc (1);
    *dataFile = (char *) malloc (1);

    //primero obtenemos el id
    while (buffer[counter] != '&') {
        idString[counter] = buffer[counter];
        counter++;
        idString = (char* ) realloc (idString, counter + 1);
    }
    idString[counter] = '\0';
    //printF(idString);
    *id = atoi(idString);
    freeString(&idString);

    counter++; //saltamos &

    //obtenemos datos 
    while ((counter < lengthData) || (buffer[counter] != '~')) {
        (*dataFile)[i] = buffer[counter];
        i++;
        counter++;
        *dataFile = (char* ) realloc (*dataFile, i + 1);
    }
    (*dataFile)[i] = '\0';
    
    //printF(*dataFile);
}

void createMP3FileInDirectory(char* directory, DescargaBowman *mythread) {
    int id = 0; 
    char* dataFile = NULL;
    char buffer[256];
    ssize_t bytesLeidos = 0;

    size_t len = strlen(directory) + strlen(mythread->song.nombre) + 2;
    char *path = malloc(len);
    snprintf(path, len, "%s/%s", directory, mythread->song.nombre);

    // Creamos el archivo .mp3
    int fd_file = open(path, O_CREAT | O_RDWR, 0644); // Apertura para leer y escribir
    if (fd_file == -1) {
        perror("Error al crear el archivo");
        freeString(&path);
        freeString(&dataFile);
        sig_func();
    }

    do {
        bytesLeidos = read(dBowman.fdPoole, buffer, 256);
        if (bytesLeidos == -1) {
            perror("Error al leer desde el file descriptor de Poole");
            break;
        }
        getIdData(buffer, &id, &dataFile);
        //printF(dataFile);
        if (write(fd_file, dataFile, bytesLeidos) == -1) { // Escribir lo leído en el archivo
            perror("Error al escribir en el archivo");
            break;
        }
        //actualizamos porcentaje y bytesdescargados
        mythread->song.bytesDescargados += strlen(dataFile);
        mythread->porcentaje = ((mythread->song.bytesDescargados)/(mythread->song.size)) * 100;

        freeString(&dataFile);
    } while (bytesLeidos > 0); 

    //COMPROBACIÓN MD5SUM
    char *md5sum = resultMd5sumComand(path);
    if (md5sum != NULL) {
        if (strcmp(md5sum, mythread->song.md5sum) == 0) {
            //OK
            setTramaString(TramaCreate(0x05, "CHECK_OK", ""), dBowman.fdPoole);
        } else {
            //KO
            setTramaString(TramaCreate(0x05, "CHECK_KO", ""), dBowman.fdPoole);
        }
        freeString(&md5sum);
    }

    close(fd_file);
    freeString(&path);
}

void downloadSong(DescargaBowman *mythread) {
    char aux[257], valorFinal = ' ';
    int inicio = 0, i = 1;

    setTramaString(TramaCreate(0x03, "DOWNLOAD_SONG", mythread->nombreDescargaComando), dBowman.fdPoole);
    ssize_t bytesLeidos = read(dBowman.fdPoole, aux, 256);
    aux[256] = '\0';

    checkPooleConnection(bytesLeidos);

    char *dataSong = read_until_string(&aux[11], '~');

    while (valorFinal != '\0') {
        char *paramDataSong = readUntilFromIndex(dataSong, &inicio, '&', &valorFinal, '\0');
        switch (i) {
            case 1: mythread->song.nombre = strdup(paramDataSong);
                break;
            case 2: mythread->song.size = atoi(paramDataSong);
                break;
            case 3: mythread->song.md5sum = strdup(paramDataSong);
                break;
            case 4: mythread->song.id = atoi(paramDataSong);
                break;
        }
        freeString(&paramDataSong);
        i++;
    }    
    freeString(&dataSong);
    
    createMP3FileInDirectory(dBowman.clienteName, mythread);
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
    dBowman.numDescargas++;

    if (pthread_create(&dBowman.descargas[dBowman.numDescargas - 1].thread, NULL, thread_function_download_song, (void *)&dBowman.descargas[dBowman.numDescargas - 1]) != 0) {
        perror("Error al crear el thread para la descarga");
        dBowman.numDescargas--;
    }

    freeString(&song);
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

            //CREAR DIRECTORIO BOWMAN
            createDirectory(dBowman.clienteName);

            printInfoFileBowman();

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
                        printF("You have no ongoing or finished downloads\n");
                    } else if (strcmp(dBowman.upperInput, "CLEAR DOWNLOADS") == 0) {
                        printF("No downloads to clear available\n");
                    } else if (strstr(dBowman.upperInput, "DOWNLOAD") != NULL) {  //DOWNLOAD <SONG/PLAYLIST>
                        //comprobar 2 arguments --> 1 espai a la comanda
                        int numSpaces = checkDownloadCommand(dBowman.upperInput);
                        if (numSpaces == 1) {
                            //NUM ARGUMENTS CORRECTE!
                            printF(dBowman.upperInput);
                            int typeFile = songOrPlaylist(dBowman.upperInput);

                            char *nombreArchivoCopia = NULL;

                            char *nombreArchivo = strchr(dBowman.input, ' ');
                            if (nombreArchivo != NULL) {
                                size_t tamano = strlen(nombreArchivo + 1) + 1;

                                nombreArchivoCopia = malloc(tamano);
                                strcpy(nombreArchivoCopia, nombreArchivo + 1); // Copia desde el carácter después del espacio
                            }

                            if (typeFile == 1) {
                                threadDownloadSong(nombreArchivoCopia);
                                printF("Download started!\n");

                            } else if (typeFile == 0) {
                                //requestDownloadPlaylist(nombreArchivoCopia);
                                printF("Download started!\n");
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