#include "Global.h"

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

//TODO FUNCION QUE QUITE '&' SI EL NOMBRE DEL CLIENTE

int main(int argc, char ** argv) {
    char *msg = NULL;
    char *input = NULL;
    char *upperInput = NULL;
    
    char *clienteName = NULL;
	char *pathClienteFile = NULL;
    char *ip = NULL;
    char *puerto = NULL;

    if (argc != 2) {
        printF("ERROR. Number of arguments is not correct\n");
        exit(EXIT_FAILURE);
    } else {
        int fd = open(argv[1], O_RDONLY);
        if (fd == -1) {
            printF("ERROR. Could not open user's file\n");
            exit(EXIT_FAILURE);
        } else {
            clienteName = read_until(fd, '\n');
            printString(clienteName);
            pathClienteFile = read_until(fd, '\n');
            printString(pathClienteFile);
            ip = read_until(fd, '\n');
            printString(ip);
            puerto = read_until(fd, '\n');
            printString(puerto);
            
            asprintf(&msg, "%s user initialized\n", clienteName);
            printF(msg);
            free(msg);

            while (1) {
                input = (char*) malloc(sizeof(char) * 100);
                memset(input, 0, 100);
                
                read(0, input, 100);
                input[strlen(input) - 1] = '\0';

                upperInput = to_upper(input);
                asprintf(&msg, "user input: %s\n", upperInput);
                printF(msg);
                free(msg);
                

                if (strcmp(upperInput, "$ LOGOUT") == 0) {
                    printF("Thanks for using HAL 9000, see you soon, music lover!\n");
                    break;
                } else if (strcmp(upperInput, "$ CONNECT") == 0) {
                    asprintf(&msg, "%s connected to HAL 9000 system, welcome music lover!\n", clienteName);
                    printF(msg);
                    free(msg);
                } else if (strcmp(upperInput, "$ LIST SONGS") == 0) {
                    printF("There are no songs available for download\n");
                } else if (strcmp(upperInput, "$ LIST PLAYLISTS") == 0) {
                    printF("There are no lists available for download\n");
                } else if (strcmp(upperInput, "$ CHECK DOWNLOADS") == 0) {
                    printF("You have no ongoing or finished downloads\n");
                } else if (strcmp(upperInput, "$ CLEAR DOWNLOADS") == 0) {
                    printF("No downloads to clear available\n");
                } else if (strstr(upperInput, "DOWNLOAD") != NULL) {  //DOWNLOAD <SONG/PLAYLIST>
                    printF("Download started!\n");
                } else {
                    printF("Unknown command\n");
                }

                free(input);
                free(upperInput);
            }


            free(upperInput);
            upperInput = NULL;
            msg = NULL;
            free(input);
            input = NULL;
            free(clienteName);
            clienteName = NULL;
            free(pathClienteFile);
            pathClienteFile = NULL;
            free(ip);
            ip = NULL;
            free(puerto);
            puerto = NULL;
            
            close(fd);
        }
    }
    return 0;
}