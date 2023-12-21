/*
Autores:
    Alba Mayol Lozano -->alba.mayol
    Kevin Eljarrat Ohayon --> kevin.eljarrat
*/

#include "Trama.h"

dataBowman dBowman;

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
}
/*if(dBowman.clientConnected) {   //Si bowman està connectat a poole
        if(!requestLogout()) {
            printF("Couldn't detach from Poole connection\n");
        }
    }*/
/*
@Finalitat: Manejar la recepción de la signal (SIGINT) y liberar los recursos utilizados hasta el momento.
@Paràmetres: ---
@Retorn: ---
*/
void sig_func() {
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
@Finalitat: Eliminar espacios en blanco adicionales
@Paràmetres: char*: str, comanda recibida
@Retorn: ---
*/
void removeExtraSpaces(char *comanda) { 
    int espacios = 0, j = 0;

    for (size_t i = 0; i < strlen(comanda); i++) {
        if (comanda[i] == ' ') {
            espacios++;
        } else {
            espacios = 0;
        }

        if (espacios <= 1) {
            comanda[j] = comanda[i];
            j++;
        }
    }
    comanda[j] = '\0';
}

/*
@Finalitat: Convertir una string a todo mayusculas.
@Paràmetres: char*: str, comando a modificar.
@Retorn: char* con el comando introducido por el usuario pasado a mayusculas.
*/
char * to_upper(char * str) {
	int length = strlen(str) + 1 ;
    char * result = (char *) malloc(length * sizeof(char));
    // inits a '\0'
	memset(result,0, length);

    for (int i = 0; i < length; i++){
        result[i] = toupper(str[i]);
    }

    return result;
}

/*
@Finalitat: Devuelve el número de espacios que hay en una string, en este caso le pasamos una comanda
@Paràmetres: char*: str, string a contar.
@Retorn: int --> número de espacios de la string
*/
int checkDownloadCommand(char * input) {
    int length = strlen(input) + 1 ;
    int numSpaces = 0;
    int i = 0;

    for (i = 0; i < length; i++) {
        if (input[i] == ' ') {
            numSpaces++;
        }
    }
    return numSpaces;
}

/*
@Finalitat: Limpiar los posibles & que pueda contener la string.
@Paràmetres: char*: clienteNameAux, string con el nombre del cliente leido del configBowman.txt.
@Retorn: char* con el nombre del usuario limpio de &.
*/
char * verifyClientName(char * clienteNameAux) {
    char *clienteName = (char *) malloc (sizeof(char));

    size_t j = 1, i;

    for (i = 0; i < strlen(clienteNameAux); i++) {
        if (clienteNameAux[i] != '&') {
            clienteName[j - 1] = clienteNameAux[i];
            j++;
            clienteName = (char *) realloc (clienteName, j * sizeof(char));
        }        
    } 
    clienteName[j - 1] = '\0';
    return clienteName;
}

/*
@Finalitat: Printa la información leída de configBowman
@Paràmetres: ---
@Retorn: ---
*/
void printInfoFile() {
    printF("\nFile read correctly:\n");
    asprintf(&dBowman.msg, "User - %s\nDirectory - %s\nIP - %s\nPort - %s\n\n", dBowman.clienteName, dBowman.pathClienteFile, dBowman.ip, dBowman.puerto);
    printF(dBowman.msg);
    freeString(&dBowman.msg);
}

/*
@Finalitat: Comprobar las posibles casuisticas con el comando DOWNLOAD
@Paràmetres: char*: downloadPtr, puntero al primer caracter, es decir a la 'D'
@Retorn: ---
*/
void checkDownload(char *downloadPtr) {
    char *mp3Ptr = strstr(downloadPtr + 10, ".MP3");
    if (mp3Ptr != NULL) {
        char nextChar = mp3Ptr[4];
        if (nextChar == '\0') {
            printF("Download started!\n");
        } else {
            printF("Please specify a single .mp3 file\n");
        }
    } else {
        printF("Please specify an .mp3 file\n");
    }
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
        dBowman.clientConnected = 1;
        asprintf(&dBowman.msg, "%s connected to HAL 9000 system, welcome music lover!\n", dBowman.clienteName);
        printF(dBowman.msg);
        freeString(&dBowman.msg);

        //freeElement(&dBowman.pooleConnected); //?????
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
}

void requestListSongs() {
    setTramaString(TramaCreate(0x02, "LIST_SONGS", ""), dBowman.fdPoole);
    
    int valorInicial = 0, numCanciones = 0, i = 0;
    int *inicio = &valorInicial; 

    char valorFinal = ' ', aux[257];
    char *final = &valorFinal;

    char *song = NULL;
    char *songs = NULL;

    char **canciones = NULL;
    
    read(dBowman.fdPoole, aux, 256);
    aux[256] = '\0';

    size_t totalSize = 0; 

    int numTramas = atoi(aux + 17);

    juntarTramasSongs(numTramas);

    while(i < numTramas) {
        read(dBowman.fdPoole, aux, 256);
        aux[256] = '\0';

        int dataSize = strlen(aux + 17);

        songs = realloc(songs, totalSize + dataSize + 1);
        if (songs == NULL) {
            break;
        }

        // Copiamos los datos de la trama actual a songs
        memcpy(songs + totalSize, aux + 17, dataSize);
        totalSize += dataSize;

        songs[totalSize] = '\0';
        i++;
    }

    do {
        song = readUntilFromIndex(songs, inicio, '&', final, '~');

        canciones = realloc(canciones, (numCanciones + 1) * sizeof(char *));
        if (canciones == NULL) {
            break;
        }
        canciones[numCanciones] = song;
        numCanciones++;
    } while (valorFinal != '~');

    asprintf(&dBowman.msg, "\nThere are %d songs available for download:", numCanciones);
    printF(dBowman.msg);
    freeString(&dBowman.msg);

    for (int i = 0; i < numCanciones; i++) {
        if (i == numCanciones - 1) {
            asprintf(&dBowman.msg, "\n%d. %s\n\n", i + 1, canciones[i]);
        } else {
            asprintf(&dBowman.msg, "\n%d. %s", i + 1, canciones[i]);
        }

        printF(dBowman.msg);
        freeString(&dBowman.msg);
        freeString(&canciones[i]);
    }

    freeString(canciones);
    freeString(&songs);
}

char *juntarTramas(int numTramas) {
    int i = 0;
    char aux[257];
    char *playlists = NULL;
    size_t totalSize = 0; 

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

char ***procesarTramas(char *playlists, int **numCancionesPorLista, int numCanciones, int *numListas) {
    int valorInicial = 0, inicialValor = 0, i = 0, totalCanciones = 0;

    int *inicioPlaylist = &valorInicial; 
    int *inicioSong = &inicialValor; 

    char valorFinal = ' ';
    char *final = &valorFinal;

    char ***listas = NULL;

    char *playlist = NULL;
    char *song = NULL;

    do {
        playlist = readUntilFromIndex(playlists, inicioPlaylist, '#', final, '~');
        //list1&song1&song2&song3\0

        size_t len = strlen(playlist);
        playlist = realloc(playlist, len + 2);
        if (playlist == NULL) {
            break;
        }

        playlist[len] = '#';
        playlist[len + 1] = '\0';

        //list1&song1&song2&song3#

        //listN&song1&song2~#

        *inicioSong = 0; 
        i = 0;
        valorFinal = ' ';
        do {    
            song = readUntilFromIndex(playlist, inicioSong, '&', final, '#');
            if (i == 0) {
                // Primero, reservamos memoria para almacenar una nueva lista
                listas = realloc(listas, (*numListas + 1) * sizeof(char **));
                if (listas == NULL) {
                    break;
                }
                // Luego, reservamos memoria para el nombre de la lista (el primer elemento de la lista)
                listas[*numListas] = malloc(sizeof(char *));
                if (listas[*numListas] == NULL) {
                    break;
                } 
                // Guardamos el nombre de la lista
                listas[*numListas][0] = strdup(song); 
                if (listas[*numListas][0] == NULL) {
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
        } while (valorFinal != '#');
        totalCanciones += (*numCancionesPorLista)[*numListas];
        (*numListas)++;

        (*numCancionesPorLista) = realloc((*numCancionesPorLista), (*numListas + 1) * sizeof(int));
        (*numCancionesPorLista)[(*numListas)] = 0;
        if (*numCancionesPorLista == NULL) {
            break;
        }
    } while (totalCanciones < numCanciones);

    freeString(&playlist);
    freeString(&song);

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
    setTramaString(TramaCreate(0x02, "LIST_PLAYLISTS", ""), dBowman.fdPoole);

    // GESTIONAR LA RECEPCION DE PLAYLISTS  
    int numListas = 0;
    int *pnumListas = &numListas;
    
    int *numCancionesPorLista = malloc(sizeof(int)); 

    *numCancionesPorLista = 0;
    
    char aux[257];

    char *playlists = NULL;
    char ***listas = NULL;
    
    // Lectura cantidad de tramas
    read(dBowman.fdPoole, aux, 256);
    aux[256] = '\0';
    int numTramas = atoi(aux + 17);

    // Lectura cantidad de canciones
    read(dBowman.fdPoole, aux, 256);
    aux[256] = '\0';
    int numCanciones = 0;
    numCanciones = atoi(aux + 17);

    playlists = juntarTramas(numTramas);

    listas = procesarTramas(playlists, &numCancionesPorLista, numCanciones, pnumListas);

    printarPlaylists(numListas, listas, numCancionesPorLista);

    freeString(&playlists);
}

int requestLogout() {  
    //HAY QUE VOLVER A CREAR OTRO SOCKET CON DISCOVERY
    setTramaString(TramaCreate(0x06, "EXIT", dBowman.clienteName), dBowman.fdPoole);
    Trama trama = readTrama(dBowman.fdPoole);
    printF(trama.header);
    if (strcmp(trama.header, "CONOK") == 0) {
        //OK
        close(dBowman.fdPoole);
        freeTrama(&trama);
        return 1;
    } else if (strcmp(trama.header, "CONKO")) {
        //KO
        printF("Sorry, couldn't logout, try again\n");
        freeTrama(&trama);
        return 0;
    }
    return 2;
}

/*
@Finalitat: Implementar el main del programa.
@Paràmetres: ---
@Retorn: int: Devuelve 0 en caso de que el programa haya finalizado exitosamente.
*/
int main(int argc, char ** argv) {
    dBowman.clientConnected = 0;
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

            printInfoFile();

            while (1) {
                printF("$ ");
                dBowman.input = read_until(0, '\n');
                dBowman.input[strlen(dBowman.input)] = '\0';
                dBowman.upperInput = to_upper(dBowman.input);
                removeExtraSpaces(dBowman.upperInput);

                if (!dBowman.clientConnected) {
                    if (strcmp(dBowman.upperInput, "CONNECT") == 0) {
                        establishDiscoveryConnection();
                        establishPooleConnection();
                    } else {
                        printF("You must establish a connection with the server before making any request\n");
                    }
                } else {    //TRANSMISIONES DISCOVERY->BOWMAN
                    if (strcmp(dBowman.upperInput, "LOGOUT") == 0) {
                        if(requestLogout()) {
                            printF("Thanks for using HAL 9000, see you soon, music lover!\n");
                            break;
                        }
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
                            printF("Download started!\n");
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