#include "Global.h"


char *msg = NULL;
char *input = NULL;
char *upperInput = NULL;

char *clienteNameAux = NULL;
char *clienteName = NULL;
char *pathClienteFile = NULL;
char *ip = NULL;
char *puerto = NULL;

void sig_func() {
    write(1, "Dentro funcion signal\n", strlen("Dentro funcion signal\n"));
    if(upperInput != NULL) {
        free(upperInput);
        upperInput = NULL;
    }
    if(msg != NULL) {
        free(msg);
        msg = NULL;
    }
    if(input != NULL) {
        free(input);
        input = NULL;
    }
    if(clienteName != NULL) {
        free(clienteName);
        clienteName = NULL;
    }
    if(clienteNameAux != NULL) {
        free(clienteNameAux);
        clienteNameAux = NULL;
    }
    if(pathClienteFile != NULL) {
        free(pathClienteFile);
        pathClienteFile = NULL;
    }
    if(ip != NULL) {
        free(ip);
        ip = NULL;
    }
    if(puerto != NULL) {
        free(puerto);
        puerto = NULL;
    }
    exit(EXIT_FAILURE);
}

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

char * verifyClientName(char * clienteNameAux) {
    char *clienteName = (char *) malloc (sizeof(char));

    size_t j = 1, i;

    for (i = 0; i < strlen(clienteNameAux); i++) {
        if (clienteNameAux[i] != '&') {
            clienteName[j - 1] = clienteNameAux[i];
            printf("%c\n", clienteName[j - 1]);
            j++;
            clienteName = (char *) realloc (clienteName, j * sizeof(char));
        }        
    } 
    clienteName[j - 1] = '\0';
    return clienteName;
}

//TODO FUNCION QUE QUITE '&' SI EL NOMBRE DEL CLIENTE

int main(int argc, char ** argv) {

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
            clienteNameAux = read_until(fd, '\n');

            clienteName = verifyClientName(clienteNameAux);
            free(clienteNameAux);
            clienteNameAux = NULL;
            

            pathClienteFile = read_until(fd, '\n');
            ip = read_until(fd, '\n');
            puerto = read_until(fd, '\n');
            
            asprintf(&msg, "%s user initialized\n", clienteName);
            printF(msg);
            free(msg);
            msg = NULL;

            while (1) {
                input = (char*) malloc(sizeof(char) * 100);
                memset(input, 0, 100);
                
                read(0, input, 100);
                input[strlen(input) - 1] = '\0';

                upperInput = to_upper(input);
                
                if (strcmp(upperInput, "$ LOGOUT") == 0) {
                    printF("Thanks for using HAL 9000, see you soon, music lover!\n");
                    break;
                } else if (strcmp(upperInput, "$ CONNECT") == 0) {
                    asprintf(&msg, "%s connected to HAL 9000 system, welcome music lover!\n", clienteName);
                    printF(msg);
                    free(msg);
                    msg = NULL;
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
                input = NULL;
                free(upperInput);
                upperInput = NULL;
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