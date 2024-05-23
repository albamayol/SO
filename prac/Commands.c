/*
Autors:
    Alba Mayol Lozano -->alba.mayol
    Kevin Eljarrat Ohayon --> kevin.eljarrat
    LaSalle - Sistemes Operatius
Data creació: 23/05/24
Data última modificació: 23/05/24
*/

#include "Commands.h"

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
@Finalitat: Comprovar les possibles casuistiques amb la comanda DOWNLOAD
@Paràmetres: char*: downloadPtr, punter al primer caràcter, es a dir, a la 'D'
@Retorn: ---
*/
void checkDownload(char *downloadPtr) {
    char *mp3Ptr = strstr(downloadPtr + 10, ".MP3");
    if (mp3Ptr != NULL) {
        char nextChar = mp3Ptr[4];
        if (nextChar == '\0') {
            write(1, "Download started!\n", strlen("Download started!\n"));
        } else {
            write(1, "Please specify a single .mp3 file\n", strlen("Please specify a single .mp3 file\n"));
        }
    } else {
        write(1, "Please specify an .mp3 file\n", strlen("Please specify an .mp3 file\n"));
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