/*
Autors:
    Alba Mayol Lozano -->alba.mayol
    Kevin Eljarrat Ohayon --> kevin.eljarrat
    LaSalle - Sistemes Operatius
Data creació: 23/05/24
Data última modificació: 23/05/24
*/

#include "FilesCreation.h"

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