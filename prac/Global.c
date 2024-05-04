/*
Autores:
    Alba Mayol Lozano -->alba.mayol
    Kevin Eljarrat Ohayon --> kevin.eljarrat
    LaSalle - Sistemes Operatius
*/

#include "Global.h"

void freeElement(Element* e) {
    if (e->name != NULL) {
        free(e->name);
        e->name = NULL;
    }

    if (e->ip != NULL) {
        free(e->ip);
        e->ip = NULL;
    }
}

void freeString(char **string) {
    if (*string != NULL) {
        free(*string);
        *string = NULL;
    }
}

void freePoolesArray(Element *array, int size) {
    for (int i = 0; i < size; ++i) {
        freeElement(&array[i]);
    }
    free(array);
}

void cleanThreadsPoole(ThreadPoole** threads, int numThreads) {
    printF("\nYou are waiting for the completion of the sending of the songs in progress to exit the program.\n");

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
    freeString(&(thread->user_name));
    
}

void cleanAllTheThreadsBowman(Descarga **descargas, int numDescargas) {
    if (numDescargas != 0) {
        printF("\nYou are waiting for the completion of the downloads of the songs in progress to exit the program\n");
    }

    for (int i = 0; i < numDescargas; i++) {
        //printf("Processing index %d\n", i);
        if ((*descargas)[i].nombreCancion != NULL) {
            //printf("Thread ID: %lu\n", (*descargas)[i].thread_id);
            pthread_join((*descargas)[i].thread_id, NULL);
            pthread_cancel((*descargas)[i].thread_id);
            freeString(&(*descargas)[i].nombreCancion);
            freeString(&(*descargas)[i].nombrePlaylist);
        }
    }
    //printf("Finished processing threads\n");
    free(*descargas);
}

void cleanInfoPlaylists(InfoPlaylist *infoPlaylists, int size) {
    for (int i = 0; i < size; ++i) {
        freeString(&(infoPlaylists[i].nameplaylist));
    }
    free(infoPlaylists);
}

void cleanThreadsBowman(Descarga **descargas, int *numDescargas, int *maxDesc) { 
    //DescargaBowman *descargasAux = NULL;
    int numDescargasAux = *numDescargas;
    *maxDesc = 0;
    //int numDescargasUpdated = 0;

    for (int i = 0; i < numDescargasAux; i++) {
        if ((*descargas)[i].porcentaje == 100) {
            pthread_cancel((*descargas)[i].thread_id);
            pthread_join((*descargas)[i].thread_id, NULL);
            freeString(&(*descargas)[i].nombreCancion);
            freeString(&(*descargas)[i].nombrePlaylist);
        } /*else {
            descargasAux = realloc(descargasAux, sizeof(DescargaBowman) * (numDescargasUpdated + 1));
            descargasAux[numDescargasUpdated].thread = (*descargas)[i].thread;
            descargasAux[numDescargasUpdated].porcentaje = (*descargas)[i].porcentaje;
            descargasAux[numDescargasUpdated].nombreDescargaComando = strdup((*descargas)[i].nombreDescargaComando);
            descargasAux[numDescargasUpdated].song.md5sum = strdup((*descargas)[i].song.md5sum);
            descargasAux[numDescargasUpdated].song.nombre = strdup((*descargas)[i].song.nombre);
            descargasAux[numDescargasUpdated].song.pathSong = strdup((*descargas)[i].song.pathSong);
            descargasAux[numDescargasUpdated].song.id = (*descargas)[i].song.id;
            descargasAux[numDescargasUpdated].song.size = (*descargas)[i].song.size;
            descargasAux[numDescargasUpdated].song.bytesDescargados = (*descargas)[i].song.bytesDescargados;
            numDescargasUpdated++;
            // 2, 5*/
        //}
    }
    /*cleanAllTheThreadsBowman(descargas, *numDescargas);

    *descargas = descargasAux;
    *numDescargas = numDescargasUpdated;*/
}

char* read_until(int fd, char delimiter) {
    char *msg = NULL;
    char current;
    int i = 0;

    while(read(fd, &current, 1) > 0) {
        if (i == 0) {
            msg = (char *) malloc(1);
        }

        if (current != delimiter) { 
            msg[i] = current;
            msg = (char *) realloc(msg, ++i + 1);
        } else {
            msg[i] = '\0';
            break;
        }
    }

    return msg;
}


char* read_until_string(char *string, char delimiter) {
    int i = 0;
    char *msg = NULL;

    while (string[i] != delimiter && string[i] != '\0') {
        msg = realloc(msg, i + 2);
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


char* readNumChars(char *string, int inicio, int final) {
    char *msg = (char *)malloc(final + 1); 
    if (msg == NULL) {
        return NULL;
    }

    for (int i = 0; i < final; i++) {
        msg[i] = string[inicio + i];
    }

    msg[final] = '\0';     
    
    return msg;
}

char* readUntilFromIndex(char *string, int *inicio, char delimiter, char *final, char delimitadorFinal) {
    char *msg = NULL;
    int i = 0;

    for (i = 0; string[(*inicio) + i] != delimitadorFinal; i++) {
        if (i == 0) {
            msg = (char *)malloc(1);
            if (msg == NULL) {
                return NULL;
            }
        }

        if (string[(*inicio) + i] != delimiter) {
            msg[i] = string[(*inicio) + i];
            msg = (char *)realloc(msg, i + 2); 
        } else {    
            msg[i] = '\0';
            *inicio += i + 1;
            return msg; 
        }
    }
    msg[i] = '\0';
    *final = delimitadorFinal;

    return msg;
}

int min(size_t a, size_t b) {
    if (a < b) {
        return a;
    }
    return b;
}

int getRandomID() { //SIEMPRE NOS GENERA LOS MISMOS ID'S
    //srand(time(NULL)); ERROR A COMENTAR EN LA MEMORIA
    return (rand() % 999) + 1; 
}

void cleanPadding(char* string, char delimiter) {
    for (size_t i = 0; i < strlen(string); i++) {
        if (string[i] == delimiter) {
            string[i] = '\0';
        }
    }
}

char *resultMd5sumComand(char *pathName) {
    int fd[2];
    char *checksum = malloc(sizeof(char) * 33);

    if (pipe(fd) == -1) {
        perror("Creacion Pipe sin exito.");
        free(checksum);
        return NULL;
    }

    pid_t childPid = fork();
    if (childPid == -1) {
        perror("Creacion fork sin exito.");
        close(fd[0]); 
        close(fd[1]); 
        free(checksum);
        return NULL;
    }

    if (childPid == 0) {
        close(fd[0]);

        char *command = NULL;
        asprintf(&command, "md5sum %s", pathName);

        close(STDOUT_FILENO);

        dup2(fd[1], STDOUT_FILENO); 
        
        execl("/bin/sh", "sh", "-c", command, (char *)NULL);

        free(command);
        close(fd[1]);
        
        exit(EXIT_SUCCESS);
    } else {
        close(fd[1]);

        read(fd[0], checksum, sizeof(char) * 32);
        checksum[32] = '\0';
        
        close(fd[0]);
    }

    return checksum;
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
	size_t length = strlen(str) + 1;
    char * result = (char *) malloc(length * sizeof(char));
	memset(result,0, length);

    for (size_t i = 0; i < length; i++){
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
    size_t length = strlen(input) + 1;
    int numSpaces = 0;
    size_t i = 0;

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

int songOrPlaylist(char *string) {
    size_t length = strlen(string);
    int indicePunto = length - 4; 

    if (strcmp(&string[indicePunto], ".MP3") == 0) {
        return 1;
    } else {
        if (strchr(string, '.') != NULL) {
            // No es un .mp3, es otra extension
            return 2;
        }
        // Es una playlist
        return 0;
    }
}

void createDirectory(char* directory) {
    struct stat st = {0};

    if (stat(directory, &st) == -1) {
        if (mkdir(directory, 0777) == -1) {
            perror("Error al crear el mkdir");
        }
    }
}

void createStatsFile(char* directory) {
    size_t len = strlen(directory) + strlen("stats.txt") + 2; 
    char *path = malloc(len);
    snprintf(path, len, "%s/%s", directory, "stats.txt");

    int fd_file = open(path, O_CREAT | O_EXCL, 0644);
    if (fd_file == -1) {
        perror("Error al crear el archivo");
        freeString(&path);
    }
    close(fd_file);
}

Element pooleMinConnections(Element *poole_list, int poole_list_size) {
    Element e;
    e.name = NULL, e.ip = NULL;
    e.port = 0;
    e.num_connections = -1;
    int j = 0;

    int minConnections = INT_MAX;
    char *buffer = NULL;
    
    if (poole_list_size != 0) {
        for (int i = 0; i < poole_list_size; i++) {
            write(1, poole_list[i].name, strlen(poole_list[i].name));
            write(1, poole_list[i].ip, strlen(poole_list[i].ip));
            
            asprintf(&buffer, "\npuerto: %d conexiones: %d\n",poole_list[i].port, poole_list[i].num_connections);
            printF(buffer);
            freeString(&buffer);

            if (poole_list[i].num_connections <= minConnections) {
                e.name = strdup(poole_list[i].name);
                e.ip = strdup(poole_list[i].ip);
                e.port = poole_list[i].port;
                e.num_connections = poole_list[i].num_connections;
                minConnections = poole_list[i].num_connections;
                write(1, e.name, strlen(e.name));
                write(1, e.ip, strlen(e.ip));
                j = i;
            
                asprintf(&buffer, "\npuerto: %d conexiones: %d minConnections: %d\n",e.port, e.num_connections, minConnections);
                printF(buffer);
                freeString(&buffer);
            }
        }
        poole_list[j].num_connections++;
    } 
    
    return e;
}

void printListPooles(Element *poole_list, int poole_list_size) {
    char* buffer = NULL;
    if (poole_list_size != 0) {
        for (int i = 0; i < poole_list_size; i++) {
            asprintf(&buffer, "\nLISTA ACTUAL DE POOLES:\nnombre: %s\nip: %s\npuerto: %d\nconexiones: %d\n",poole_list[i].name, poole_list[i].ip, poole_list[i].port, poole_list[i].num_connections);
            printF(buffer);
            freeString(&buffer);
        }
    } 
}

int decreaseNumConnections(Element *poole_list, int poole_list_size, char* pooleName) {
    for (int i = 0; i < poole_list_size; i++) {
        printF(poole_list[i].name);
        if (strcmp(poole_list[i].name, pooleName) == 0) {
            poole_list[i].num_connections--;
            return 1;
        }
    }
    return 0; 
}

int erasePooleFromList(Element** poole_list, int* poole_list_size, char* pooleName) {
    int flagFound = 0;
    Element* updatedPooleList = NULL;
    int updatedPooleListSize = 0;
    /*printF("pooleNameSalida: ");
    printF(pooleName);
    printF("\n");*/

    for (int i = 0; i < *poole_list_size; i++) {
        //printF((*poole_list)[i].name);
        if (strcmp((*poole_list)[i].name, pooleName) != 0) {
            updatedPooleList = realloc(updatedPooleList, sizeof(Element) * (updatedPooleListSize + 1));
            updatedPooleList[updatedPooleListSize].name = strdup((*poole_list)[i].name);
            updatedPooleList[updatedPooleListSize].ip = strdup((*poole_list)[i].ip);
            updatedPooleList[updatedPooleListSize].port = (*poole_list)[i].port;
            updatedPooleList[updatedPooleListSize].num_connections = (*poole_list)[i].num_connections;
            updatedPooleListSize++;
        } else {
            flagFound = 1;
        }
    }
    freePoolesArray(*poole_list, *poole_list_size);
    *poole_list_size = updatedPooleListSize;
    *poole_list = updatedPooleList;
    char *prueba = NULL;
    asprintf(&prueba, "FLAG FOUND: %d", flagFound);
    printF(prueba);
    printF("\n");
    freeString(&prueba);
    return flagFound; 
}

char* convertIntToString(int num) {
    int numDigits = snprintf(NULL, 0, "%d", num);  
    char* string = (char*)malloc(sizeof(char)*(numDigits + 1)); 
    sprintf(string, "%d", num); 

    return string;
}

void separaDataToElement(char* data, Element* e) {
    int i = 0, j = 0;
    char *name = NULL, *ip = NULL, *port = NULL; 

    name = (char *)malloc(sizeof(char));
    ip = (char *)malloc(sizeof(char));
    port = (char *)malloc(sizeof(char));

    while(data[i] != '&') {
        name[j] = data[i];
        j++;
        name = (char *) realloc(name, j + 1);
        i++;
    }
    name[j] = '\0';

    //saltamos primer '&'
    i++;
    j = 0;

    while(data[i] != '&') {
        ip[j] = data[i];
        j++;
        ip = (char *) realloc(ip, j + 1);
        i++;
    }
    ip[j] = '\0';
    //saltamos segundo '&'
    i++;
    j = 0;


    while(data[i] != '\0') {
        port[j] = data[i];
        j++;
        port = (char *) realloc(port, j + 1);
        i++;
    }
    port[j] = '\0';

    e->name = strdup(name);
    e->ip = strdup(ip);
    e->port = atoi(port);
    e->num_connections = 0;

    freeString(&name);
    freeString(&ip);
    freeString(&port);
}