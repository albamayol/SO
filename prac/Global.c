/*
Autores:
    Alba Mayol Lozano -->alba.mayol
    Kevin Eljarrat Ohayon --> kevin.eljarrat
    LaSalle - Sistemes Operatius
*/

#include "Global.h"

/*
@Finalitat: Alliberar la memòra d'un Element de la llista de Poole's de Discovery
@Paràmetres: Element* e: Element a alliberar
@Retorn: ---
*/
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

/*
@Finalitat: Alliberar la memòria d'un string dinàmic
@Paràmetres: char** string: string a alliberar
@Retorn: ---
*/
void freeString(char **string) {
    if (*string != NULL) {
        free(*string);
        *string = NULL;
    }
}

/*
@Finalitat: Allibera cada Element de l'array de Poole's de Discovery al igual que el propi array també
@Paràmetres: Element* array: array de Poole's de Discovery
@Retorn: ---
*/
void freePoolesArray(Element *array, int size) {
    for (int i = 0; i < size; ++i) {
        freeElement(&array[i]);
    }
    free(array);
}

/*
@Finalitat: cancelar i matar els threads de Poole que ja hagin acabat les descarregues
@Paràmetres: ThreadPoole** thread: array de descarrgues de Poole; int numThreads: mida de l'array de threads de Poole (connexions de Bowman)
@Retorn: ---
*/
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
    freeString(&(thread->user_name));
    
}

/*
@Finalitat: cancelar i matar tots els threads de Bowman 
@Paràmetres: Descarga** descargas: array de descarrgues de Bowman; int* numDescargas: mida de l'array de descarregues;
@Retorn: ---
*/
void cleanAllTheThreadsBowman(Descarga **descargas, int numDescargas) {
    if (numDescargas != 0) {
        printF("\nYou are waiting for the completion of the downloads of the songs in progress to exit the program\n");
    }

    for (int i = 0; i < numDescargas; i++) {
        if ((*descargas)[i].nombreCancion != NULL) {
            pthread_join((*descargas)[i].thread_id, NULL);
            freeString(&(*descargas)[i].nombreCancion);
            freeString(&(*descargas)[i].nombrePlaylist);
        }
    }
    free(*descargas);
}

/*
@Finalitat: Neteja i allibera l'array de playlists que rep Bowman segons la llista de playlists del seu Poole
@Paràmetres: InfoPlaylist* infoPlaylists: array de playlists de Bowman; int size: mida de l'array de playlists
@Retorn: ---
*/
void cleanInfoPlaylists(InfoPlaylist *infoPlaylists, int size) {
    for (int i = 0; i < size; ++i) {
        freeString(&(infoPlaylists[i].nameplaylist));
    }
    free(infoPlaylists);
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
            freeString(&(*descargas)[i].nombreCancion);
            freeString(&(*descargas)[i].nombrePlaylist);
        } 
    }
}

/*
@Finalitat: Retorna un string generada de la lectura d'un file descriptor, es llegeix fins un delimitador donat
@Paràmetres: int fd: file desccriptor d'on llegir; char delimiter: caràcter delimitador
@Retorn: char*: string llegida
*/
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


/*
@Finalitat: Retorna un string generada de la lectura d'una string principal, que s'ha llegit fins un delimitador donat
@Paràmetres: char* string: string a llegir; char delimiter: caràcter delimitador
@Retorn: char*: string llegida
*/
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

/*
@Finalitat: Retorna un string generada de la lectura d'una string principal, que s'ha llegit des d'un index inicial donat fins un altra de final
@Paràmetres: char* string: string a llegir; int inicio: index inicial; int final: index final
@Retorn: char*: string llegida
*/
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

/*
@Finalitat: Llegeix d'una string desde un index inicial fins a un delimitador final, i retorna la cadena trobada entre ambdòs limitadors
@Paràmetres: char* string: string a llegir; int *inicio: Punter a l'índex inicial; char delimiter: caràcter que indica fins on llegir; char *final: caràcter on es guardarà el delimitador final trobat; char delimitadorFinal: caràcter que indica el final absolut
@Retorn: char*: cadena llegida
*/
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

/*
@Finalitat: Retorna el més petit de 2 valors donats
@Paràmetres: size_t a: primer valor; size_t b: segon valor
@Retorn: int: valor més petit
*/
int min(size_t a, size_t b) {
    if (a < b) {
        return a;
    }
    return b;
}

/*
@Finalitat: Genera un número aleatori
@Paràmetres: ---
@Retorn: int: número aleatori
*/
int getRandomID() { 
    return (rand() % 999) + 1; 
}

/*
@Finalitat: Neteja el padding donat d'una trama
@Paràmetres: char* string: string a netejar, char delimiter: valor del padding a netejar
@Retorn: ---
*/
void cleanPadding(char* string, char delimiter) {
    for (size_t i = 0; i < strlen(string); i++) {
        if (string[i] == delimiter) {
            string[i] = '\0';
        }
    }
}

/*
@Finalitat: Calcular el md5sum d'un fitxer segons un path donat
@Paràmetres: char* pathName: path al fitxer
@Retorn: char* :string amb el md5sum
*/
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
@Finalitat: Eliminar espais en blanc adicionals
@Paràmetres: char* str: comanda rebuda
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
@Finalitat: Convertir una string a tot majúscules.
@Paràmetres: char* str: comanda a modificar.
@Retorn: char*: comanda introduïda per l'usuari pasada a majúscules.
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
@Finalitat: Retorna el número d'espais que hi ha en una string, en aquest cas li passem una comanda
@Paràmetres: char* str: string a comptar.
@Retorn: int --> número d'espais de la string
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
@Finalitat: Netejar els possibles '&' que pugui contenir la string.
@Paràmetres: char*: clienteNameAux, string amb el nom del client llegit del configBowman.txt.
@Retorn: char*: string amb el nom de l'usuari sense '&'.
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
@Finalitat: Comprovar les possibles casuistiques amb la comanda DOWNLOAD
@Paràmetres: char*: downloadPtr, punter al primer caràcter, es a dir, a la 'D'
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

/*
@Finalitat: detectar si segons un nom, aquest es tracta d'una cançó o d'una playlist
@Paràmetres: char* string: nom a identificar
@Retorn: int: 0 si es una playlist, 1 si es un .mp3, 0 si es un altra tipus de fitxer
*/
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

/*
@Finalitat: crea un directori segons un nom donat
@Paràmetres: char* directory: nom del directori a crear
@Retorn: ---
*/
void createDirectory(char* directory) {
    struct stat st = {0};

    if (stat(directory, &st) == -1) {
        if (mkdir(directory, 0777) == -1) {
            perror("Error al crear el mkdir");
        }
    }
}

/*
@Finalitat: crea el fitxer stats.txt al directori de Poole corresponent
@Paràmetres: char* directory: path al directori on crear el fitxer
@Retorn: ---
*/
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

/*
@Finalitat: Busca el Poole amb el mínim de connexions dins els que hi ha a l'array de Poole's de Discovery
@Paràmetres: Element* poole_list: array de Poole's de Discovery; int* poole_list_size: mida de l'array de Poole's
@Retorn: Element: El Poole amb el mínim de connexions
*/
Element pooleMinConnections(Element *poole_list, int poole_list_size) {
    Element e;
    e.name = NULL, e.ip = NULL;
    e.port = 0;
    e.num_connections = -1;
    int j = 0;

    int minConnections = INT_MAX;

    if (poole_list_size != 0) {
        for (int i = 0; i < poole_list_size; i++) {
            if (poole_list[i].num_connections <= minConnections) {
                e.name = strdup(poole_list[i].name);
                e.ip = strdup(poole_list[i].ip);
                e.port = poole_list[i].port;
                e.num_connections = poole_list[i].num_connections;
                minConnections = poole_list[i].num_connections;
                j = i;
            }
        }
        poole_list[j].num_connections++;
    } 
    
    return e;
}

/*
@Finalitat: Printa la llista de Poole's de Discovery, amb la informació rellevant de cada Poole
@Paràmetres: Element* poole_list: array de Poole's de Discovery; int* poole_list_size: mida de l'array de Poole's
@Retorn: ---
*/
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

/*
@Finalitat: Decrementa en 1 el número de connexions de Bowman's d'un Poole en concret dins la llista de Poole's de Discovery
@Paràmetres: Element *poole_list: llista de Poole's; int poole_list_size: mida de la llista de Poole's; char* pooleName: nom del Poole
@Retorn: int: 0 si no s'ha trobat el Poole, 1 si s'ha trobat i decrementat el num_connexions del Poole
*/
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

/*
@Finalitat: Elimina un Poole de l'array de Poole de Discovery
@Paràmetres: Element** poole_list: array de Poole's de Discovery; int* poole_list_size: mida de l'array de Poole's; char* pooleName: nom del Poole a esborrar
@Retorn: int: 0 si no s'ha trobat aquell Poole, 1 si s'ha trobat i esborrat aquell Poole
*/
int erasePooleFromList(Element** poole_list, int* poole_list_size, char* pooleName) {
    int flagFound = 0;
    Element* updatedPooleList = NULL;
    int updatedPooleListSize = 0;

    for (int i = 0; i < *poole_list_size; i++) {
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
    
    return flagFound; 
}

/*
@Finalitat: Implementació d'un ITOA
@Paràmetres: int num: número a convertir
@Retorn: char*: string del número convertit
*/
char* convertIntToString(int num) {
    int numDigits = snprintf(NULL, 0, "%d", num);  
    char* string = (char*)malloc(sizeof(char)*(numDigits + 1)); 
    sprintf(string, "%d", num); 

    return string;
}

/*
@Finalitat: Separa el camp data en les diferents informacions del nou Poole conectat a Discovery
@Paràmetres: char* data: camp data, Element* e: estructura de dades de Poole
@Retorn: ---
*/
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