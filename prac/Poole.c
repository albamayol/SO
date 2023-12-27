/*
Autores:
    Alba Mayol Lozano -->alba.mayol
    Kevin Eljarrat Ohayon --> kevin.eljarrat
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
    //ENVIAMOS TRAMA LOGOUTBOWMAN
    setTramaString(TramaCreate(0x06, "POOLE_DISCONNECT", dPoole.serverName), dPoole.fdPooleClient);
    Trama trama = readTrama(dPoole.fdPooleClient);
    printF(trama.header);
    if (strcmp(trama.header, "CONOK") == 0) {
        //nos desconectamos bien
        printF("Poole disconnected from Discovery succesfully\n");
    } else if (strcmp(trama.header, "CONKO") == 0) {
        printF("Sorry, couldn't disconnect from Discovery\n");
    }

    freeTrama(&trama);
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
    cleanThreadsPoole(&dPoole.threads, dPoole.threads_array_size); //cancel y despues join

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

void notifyBowmanLogout(int fd_bowman) {
    openDiscoverySocket();
    //ENVIAMOS TRAMA LOGOUTBOWMAN
    setTramaString(TramaCreate(0x06, "BOWMAN_LOGOUT", dPoole.serverName), dPoole.fdPooleClient);
    Trama trama = readTrama(dPoole.fdPooleClient);
    printF(trama.header);
    if (strcmp(trama.header, "CONOK") == 0) {
        //Avisamos Bowman OK
        setTramaString(TramaCreate(0x06, "CONOK", dPoole.serverName), fd_bowman);
    } else if (strcmp(trama.header, "CONKO") == 0) {
        //Avisamos Bowman KO
        setTramaString(TramaCreate(0x06, "CONKO", dPoole.serverName), fd_bowman);
    }

    freeTrama(&trama);
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
        if (entry->d_type == DT_REG && strcmp(entry->d_name, ".DS_Store") != 0) { // Verificar si es un archivo regular
            size_t fileNameLen = strlen(entry->d_name);
            *fileNames = realloc(*fileNames, totalLength + fileNameLen + 1); // +1 para el \0
            
            if (*fileNames == NULL) {
                perror("Error en realloc");
                closedir(dir);
                return;
            }

            if (!isFirstFile) {
                strcat(*fileNames, "&"); // Agregar '&' solo si no es el primer archivo
            } else {
                isFirstFile = 0; 
            }

            strcat(*fileNames, entry->d_name);
            totalLength += fileNameLen + 1; // Longitud del nombre + 1 para '&'
            (*totalSongs) += 1;
        }
    }

    closedir(dir);
}

void enviarTramas(int fd_bowman, char *cadena) {
    int i = 0;
    char *trama = NULL;

    size_t sizeData = strlen(cadena);

    // Primero de todo enviamos una trama con el numero total de tramas que procesará Bowman.
    size_t aux = sizeData;

    while (aux > 239) {
        aux -= 239; 
        i++;
    }
    char *numTramas = convertIntToString(i + 1);
    setTramaString(TramaCreate(0x02, "SONGS_RESPONSE", numTramas), fd_bowman);
    freeString(&numTramas);

    if (sizeData < 239) { // 256 - Type(1 Byte) - header_length(2 Bytes) - Header(14 Bytes) = 239 Bytes disponibles
        trama = readNumChars(cadena, 0, sizeData);
        asprintf(&dPoole.msg,"\nTrama %d: %s.\n", i + 1, trama);
        printF(dPoole.msg);
        freeString(&dPoole.msg);
        setTramaString(TramaCreate(0x02, "SONGS_RESPONSE", trama), fd_bowman);
    } else {
        i = 0;
        while (sizeData > 239) {
            trama = readNumChars(cadena, i * 239, 239);
            asprintf(&dPoole.msg,"\nTrama %d: %s.\n", i + 1, trama);
            printF(dPoole.msg);
            freeString(&dPoole.msg);
            setTramaString(TramaCreate(0x02, "SONGS_RESPONSE", trama), fd_bowman);
            sizeData -= 239; 
            i++;
            freeString(&trama);
        }
        trama = readNumChars(cadena, i * 239, sizeData);
        asprintf(&dPoole.msg,"\nTrama %d: %s.\n", i + 1, trama);
        printF(dPoole.msg);
        freeString(&dPoole.msg);
        setTramaString(TramaCreate(0x02, "SONGS_RESPONSE", trama), fd_bowman);
    }
    
    freeString(&trama);
}

void sendListSongs(int fd_bowman) {
    char *songs = NULL; 

    int totalSongs = 0;

    listSongs(dPoole.serverName, &songs, &totalSongs);

    printF(songs);

    enviarTramas(fd_bowman, songs);

    freeString(&songs);
}

void listPlaylists(const char *path, char **fileNames, int *totalSongs) {
    struct dirent *entry;
    DIR *dir = opendir(path);

    if (dir == NULL) {
        perror("Error al abrir el directorio");
        return;
    }

    size_t totalLength = 1; // Inicializar con un byte para '\0'
    *fileNames = malloc(1);
    (*fileNames)[0] = '\0'; 

    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_type == DT_DIR && strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
            // Es un directorio y no es "." ni ".."
            size_t fileNameLen = strlen(entry->d_name);
            char *subPath = malloc(strlen(path) + fileNameLen + 2); // +2 para / y \0
            sprintf(subPath, "%s/%s", path, entry->d_name);

            char *subSongs = NULL;

            listSongs(subPath, &subSongs, totalSongs);

            size_t newLength = totalLength + fileNameLen + 1; // Longitud del nombre de archivo y el separador '&'

            if (subSongs != NULL) {
                // Si subSongs no es nulo, agrega su longitud más un carácter para el separador '&'
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

void sendListPlaylists(int fd_bowman) {
    char *playlists = NULL;
    int totalSongs = 0;

    listPlaylists(dPoole.serverName, &playlists, &totalSongs);
    printF(playlists);

    char *cantidadCanciones = convertIntToString(totalSongs);
    setTramaString(TramaCreate(0x02, "SONGS_RESPONSE", cantidadCanciones), fd_bowman);
    freeString(&cantidadCanciones);

    enviarTramas(fd_bowman, playlists);
    freeString(&playlists);
}

int searchSong(char *song, int *fileSize) {
    DIR *dir;
    struct dirent *entry;
    struct stat st;
    int found = 0;

    dir = opendir(dPoole.serverName); 
    if (dir != NULL) {
        while ((entry = readdir(dir)) != NULL) {
            if (entry->d_type == DT_REG && strcmp(entry->d_name, ".DS_Store") != 0) { 

                //size_t nameLength = strlen(entry->d_name);

                // Creamos una cadena temporal para almacenar el nombre del archivo sin los residuos
                /*char *fileName = malloc((nameLength + 1) * sizeof(char));
                strncpy(fileName, entry->d_name, nameLength);
                fileName[nameLength] = '\0';*/


                if (strcmp("Cafe_Malibú_-_Sech,_Mora,_Saiko.mp3", song) == 0) { // PROBLEMA
                    found = 1;

                    if (stat("Cafe_Malibú_-_Sech,_Mora,_Saiko.mp3", &st) == -1) {
                        perror("Error al obtener el tamaño del archivo");
                        *fileSize = 0;
                    } else {
                        *fileSize = st.st_size;
                    }
                    //freeString(&fileName);
                    break;
                }
                //freeString(&fileName);
            }
        }
        closedir(dir);
    } else {
        perror("Error al abrir el directorio");
    }
    return found;
}

char * resultMd5sumComand(char *directoryPath, char* filename) {
	//const int bufferSize = 256;
	//char command[bufferSize];

    DIR *dir = opendir(directoryPath);
    if (dir == NULL) {
        perror("opendir");
        return NULL;
    }
    
    char *command = NULL;


    asprintf(&command, "md5sum %s", filename);
    // Create the command string
    //snprintf(command, bufferSize, "md5sum %s", filename);

    FILE* pipe = popen(command, "r");
    if (pipe == NULL) {
        perror("popen");
        return NULL;
    }

    // Create a dynamic buffer to store the command output
    char* buffer = (char*)malloc(32);
    if (buffer == NULL) {
        perror("malloc");
        pclose(pipe);
        return NULL;
    }

    int bytesRead = read(fileno(pipe), buffer, 32 - 1);
    if (bytesRead == -1) {
        perror("read");
        free(buffer);
        pclose(pipe);
        return NULL;
    }

    if (bytesRead == 0) {
        buffer[0] = '\0';
    } else {
        buffer[bytesRead] = '\0';
    }

    // Close the pipe
    pclose(pipe);
    closedir(dir);
    freeString(&command);

    return buffer;
}

int getRandomID() {
    srand(time(NULL)); // Semilla basada en el tiempo actual
    return rand() % 1000; // Genera un número aleatorio entre 0 y 999
}

void enviarDatosSong(int fd_bowman, char *directoryPath, char *song, char *id) {

    DIR *dir = opendir(directoryPath);
    if (dir == NULL) {
        perror("opendir");
        return;
    }

    int fd = open(song, O_RDONLY);
    if (fd == -1) {
        perror("Error al abrir el archivo");
        return;
    }

    size_t longitudId = strlen(id);

    char *data = malloc(244); // 256 - 3(Type + Header Length) - 9(Bytes del Header) = 244 Bytes
    char *buffer = malloc(244 - longitudId - 1); 

    strcpy(data, id);
    strcat(data, "&"); 
    
    size_t bytesLeidos = 0;

    // Leer el archivo y enviar los datos
    while ((bytesLeidos = read(fd, buffer, 244 - longitudId - 1)) > 0) {
        
        strcat(data, buffer); 
        

        setTramaString(TramaCreate(0x04, "FILE_DATA", data), fd_bowman);

        strcpy(data, id);
        strcat(data, "&");

        bytesLeidos = 0;
    }
    strcat(data, buffer); 
    setTramaString(TramaCreate(0x04, "FILE_DATA", data), fd_bowman);
    //sdfjasdjsdfsj\0\0sfdsd----------------------- = 256

    free(data);
    free(buffer);

    close(fd);
}

void sendSong(char *song, int fd_bowman) {
    int fileSize = 0;

    // Si se encuentra el archivo 'song' devolvera su tamaño
    if (searchSong(song, &fileSize)) {
        // Calcular el MD5SUM
        char *md5sum = resultMd5sumComand(dPoole.serverName, song);

        if (md5sum != NULL) {
            // Calcular el numero random
            int randomID = getRandomID();

            char *data = createString4Params(song, convertIntToString(fileSize), md5sum, convertIntToString(randomID));
            printF(data);
            setTramaString(TramaCreate(0x04, "NEW_FILE", data), fd_bowman);
        
            freeString(&md5sum);
            freeString(&data);

            // Enviar los datos del fichero
            enviarDatosSong(fd_bowman, dPoole.serverName, song, convertIntToString(randomID));
        }
    
    }

    // Crear thread 
    // Cuando haya terminada liberar el thread.
}

/*void sendPlaylist(char *playlist) {
    // Buscar el nombre de la lista
    // Crear thread
    // Empezar a enviar las canciones, creo N thread por N canciones
    // Cuando haya terminado libero todos los threads.
}*/

void conexionBowman(ThreadPoole* mythread) {
    int exit = 0;

    Trama trama = readTrama(mythread->fd);
    mythread->user_name = strdup(trama.data);

    asprintf(&dPoole.msg,"\nNew user connected: %s.\n", mythread->user_name);
    printF(dPoole.msg);
    freeString(&dPoole.msg);

    // Transmisión Poole->Bowman para informar del estado de la conexion.
    if (strcmp(trama.header, "NEW_BOWMAN") == 0) {
        setTramaString(TramaCreate(0x01, "CON_OK", ""), mythread->fd);
    } else {
        setTramaString(TramaCreate(0x01, "CON_KO", ""), mythread->fd);
        close(mythread->fd);

        freeTrama(&trama);
        cleanThreadPoole(mythread); //con el join ya salimos de la funcion de thread osea salimos de aqui
    }
    
    freeTrama(&trama);

    //TRANSMISIONES POOLE-->BOWMAN
    while(1) {
        trama = readTrama(mythread->fd);

        if (strcmp(trama.header, "EXIT") == 0) {    
            notifyBowmanLogout(mythread->fd);
            
            asprintf(&dPoole.msg,"\nNew request - %s logged out\n", mythread->user_name);
            printF(dPoole.msg);
            freeString(&dPoole.msg);

            freeTrama(&trama);
            cleanThreadPoole(mythread); //clean thread hace cancel y join -> CANCEL + JOIN espera a que acabe la ejecución del thread y nos saca de la thread function
        } else if (strcmp(trama.header, "LIST_SONGS") == 0) {
            asprintf(&dPoole.msg,"\nNew request - %s requires the list of songs.\nSending song list to %s\n", mythread->user_name, mythread->user_name);
            printF(dPoole.msg);
            freeString(&dPoole.msg);

            sendListSongs(mythread->fd);
        } else if (strcmp(trama.header, "LIST_PLAYLISTS") == 0) {
            asprintf(&dPoole.msg,"\nNew request - %s requires the list of playlists.\nSending playlist list to %s\n", mythread->user_name, mythread->user_name);
            printF(dPoole.msg);
            freeString(&dPoole.msg);

            sendListPlaylists(mythread->fd);
        } else if (strcmp(trama.header, "CHECK DOWNLOADS") == 0) {
            printF("You have no ongoing or finished downloads\n");
        } else if (strcmp(trama.header, "CLEAR DOWNLOADS") == 0) {
            printF("No downloads to clear available\n");
        } else if (strstr(trama.header, "DOWNLOAD") != NULL) {  //DOWNLOAD <SONG/PLAYLIST>
            char *upperInput = to_upper(trama.data);
            removeExtraSpaces(upperInput);

            int typeFile = songOrPlaylist(upperInput);
            freeString(&upperInput);

            if (typeFile == 1) {
                sendSong(trama.data, mythread->fd);
            } else {
                //sendPlaylist(trama.data);
            }
        } else {
            printF("Unknown command\n");
        }
        printF("freedTrama\n");
        freeTrama(&trama);  
    }
    freeTrama(&trama);  
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

    dPoole.threads = realloc(dPoole.threads, sizeof(ThreadPoole) * (dPoole.threads_array_size + 1)); //alojamos una posición
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