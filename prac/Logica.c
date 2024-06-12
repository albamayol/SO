/*
Autors:
    Alba Mayol Lozano -->alba.mayol
    Kevin Eljarrat Ohayon --> kevin.eljarrat
    LaSalle - Sistemes Operatius
Data creació: 23/05/24
Data última modificació: 23/05/24
*/

#include "Logica.h"

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
@Finalitat: Cerca si una cançó existeix
@Paràmetres: char* pathSongPlaylist: string amb el path a la cançó a buscar
@Retorn: int: 0 si no s'ha trobat, 1 si s'ha trobat
*/
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

/*
@Finalitat: Cerca si una playlist existeix
@Paràmetres: char* pathSongPlaylist: string amb el path a la playlist a buscar
@Retorn: int: 0 si no s'ha trobat, 1 si s'ha trobat
*/
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

/*
@Finalitat: retorna el número d'arxius regulars d'un directori especificat
@Paràmetres: const char* path: string amb el path a la playlist demanada
@Retorn: int: número d'arxius 
*/
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

/*
@Finalitat: crea una llista de totes les cançons (fitxers .mp3) dins del directori donat i retorna els seus noms concatenats en una sola cadena.
@Paràmetres: const char *path: path del directori de les cançons; char **fileNames: cadena dels noms de les cançons concatenades;int *totalSongs: número total de cançons trobades.
@Retorn: ---
*/
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

/*
@Finalitat: Llista les playlists dins d'un directori donat i retorna els seus noms concatenats en una sola cadena.
@Paràmetres: const char *path: string amb el path del directori; char **fileNames: punter a una cadena on es retornaran els noms de les playlists concatenades; int *totalSongs: punter a un enter on es retornarà el nombre total de cançons.
@Retorn: ---
*/
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
                free(subSongs);
                subSongs = NULL;
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
            free(subSongs);
            subSongs = NULL;
            totalLength = newLength;
        }
    }

    closedir(dir);
}

/*
@Finalitat: Mira si el Poole al que està connectat Bowman segueix o no "en peu" i avisa a Bowman de desconnectar-se també
@Paràmetres: ---
@Retorn: ---
*/
void checkPooleConnection(int *bowmanConnected, char* clienteName) {
    *bowmanConnected = 0;
    char* msgAuxiliar = NULL;
    asprintf(&msgAuxiliar, "\n¡Alert: %s disconnected because the server connection has ended!\nPlease press Control C to exit the program.\n", clienteName);
    write(1, msgAuxiliar, strlen(msgAuxiliar));
    free(msgAuxiliar);
    msgAuxiliar = NULL;
}

/*
@Finalitat: Crea 2 cues de missatges per a la gestió de les trames
@Paràmetres: ---
@Retorn: ---
*/
void creacionMsgQueues(int *msgQueueDescargas, int* msgQueuePetitions) {
    key_t key1 = ftok("Bowman.c", 0xCA);

    int id_queue = msgget(key1, 0666 | IPC_CREAT);
    if (id_queue < 0) {
        write(1, "Error al crear la cua de missatges de les peticions\n", strlen("Error al crear la cua de missatges de les peticions\n"));
        return;
    }
    *msgQueuePetitions = id_queue;

    key_t key2 = ftok("Bowman.c", 0xCB);

    id_queue = msgget(key2, 0666 | IPC_CREAT);
    if (id_queue < 0) {
        write(1, "Error al crear la cua de missatges de les descargues\n", strlen("Error al crear la cua de missatges de les descargues\n"));
        return;
    }
    *msgQueueDescargas = id_queue;
}