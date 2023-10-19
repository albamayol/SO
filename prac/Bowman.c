/*
Autores:
    Alba Mayol Lozano -->alba.mayol
    Kevin Eljarrat Ohayon --> kevin.eljarrat
*/

#include "Global.h"

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

/*
@Finalitat: Manejar la recepción de la signal (SIGINT) y liberar los recursos utilizados hasta el momento.
@Paràmetres: ---
@Retorn: ---
*/
void sig_func() {
    if(dBowman.upperInput != NULL) {
        free(dBowman.upperInput);
        dBowman.upperInput = NULL;
    }
    if(dBowman.msg != NULL) {
        free(dBowman.msg);
        dBowman.msg = NULL;
    }
    if(dBowman.input != NULL) {
        free(dBowman.input);
        dBowman.input = NULL;
    }
    if(dBowman.clienteName != NULL) {
        free(dBowman.clienteName);
        dBowman.clienteName = NULL;
    }
    if(dBowman.clienteNameAux != NULL) {
        free(dBowman.clienteNameAux);
        dBowman.clienteNameAux = NULL;
    }
    if(dBowman.pathClienteFile != NULL) {
        free(dBowman.pathClienteFile);
        dBowman.pathClienteFile = NULL;
    }
    if(dBowman.ip != NULL) {
        free(dBowman.ip);
        dBowman.ip = NULL;
    }
    if(dBowman.puerto != NULL) {
        free(dBowman.puerto);
        dBowman.puerto = NULL;
    }
    exit(EXIT_FAILURE);
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
    
    printF("File read correctly:\n");
    asprintf(&dBowman.msg, "User - %s\n", dBowman.clienteName);
    printF(dBowman.msg);
    free(dBowman.msg);
    

    asprintf(&dBowman.msg, "Directory - %s\n", dBowman.pathClienteFile);
    printF(dBowman.msg);
    free(dBowman.msg);
    

    asprintf(&dBowman.msg, "IP - %s\n", dBowman.ip);
    printF(dBowman.msg);
    free(dBowman.msg);
    

    asprintf(&dBowman.msg, "Port - %s\n", dBowman.puerto);
    printF(dBowman.msg);
    free(dBowman.msg);


    dBowman.msg = NULL;
}

/*
@Finalitat: Implementar el main del programa.
@Paràmetres: ---
@Retorn: int: Devuelve 0 en caso de que el programa haya finalizado exitosamente.
*/
int main(int argc, char ** argv) {
    int clientConnected = 0;
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
            free(dBowman.clienteNameAux);
            dBowman.clienteNameAux = NULL;
            

            dBowman.pathClienteFile = read_until(fd, '\n');
            dBowman.ip = read_until(fd, '\n');
            dBowman.puerto = read_until(fd, '\n');
            
            asprintf(&dBowman.msg, "%s user initialized\n", dBowman.clienteName);
            printF(dBowman.msg);
            free(dBowman.msg);
            dBowman.msg = NULL;

            printInfoFile();

            while (1) {
                dBowman.input = (char*) malloc(sizeof(char) * 100);
                memset(dBowman.input, 0, 100);
                
                read(0, dBowman.input, 100);
                dBowman.input[strlen(dBowman.input) - 1] = '\0';

                dBowman.upperInput = to_upper(dBowman.input);
                 if (!clientConnected) {
                    if (strcmp(dBowman.upperInput, "$ CONNECT") == 0) {
                        asprintf(&dBowman.msg, "%s connected to HAL 9000 system, welcome music lover!\n", dBowman.clienteName);
                        printF(dBowman.msg);
                        free(dBowman.msg);
                        dBowman.msg = NULL;
                        clientConnected = 1;
                    } else {
                        printF("You must establish a connection with the server before making any request\n");
                    }
                } else {
                    if (strcmp(dBowman.upperInput, "$ LOGOUT") == 0) {
                        printF("Thanks for using HAL 9000, see you soon, music lover!\n");
                        break;
                    } else if (strcmp(dBowman.upperInput, "$ LIST SONGS") == 0) {
                        printF("There are no songs available for download\n");
                    } else if (strcmp(dBowman.upperInput, "$ LIST PLAYLISTS") == 0) {
                        printF("There are no lists available for download\n");
                    } else if (strcmp(dBowman.upperInput, "$ CHECK DOWNLOADS") == 0) {
                        printF("You have no ongoing or finished downloads\n");
                    } else if (strcmp(dBowman.upperInput, "$ CLEAR DOWNLOADS") == 0) {
                        printF("No downloads to clear available\n");
                    } else if (strstr(dBowman.upperInput, "DOWNLOAD") != NULL) {  //DOWNLOAD <SONG/PLAYLIST>
                        printF("Download started!\n");
                    } else {
                        printF("Unknown command\n");
                    }
                }

                free(dBowman.input);
                dBowman.input = NULL;
                free(dBowman.upperInput);
                dBowman.upperInput = NULL;
            }


            free(dBowman.upperInput);
            dBowman.upperInput = NULL;
            dBowman.msg = NULL;
            free(dBowman.input);
            dBowman.input = NULL;
            free(dBowman.clienteName);
            dBowman.clienteName = NULL;
            free(dBowman.pathClienteFile);
            dBowman.pathClienteFile = NULL;
            free(dBowman.ip);
            dBowman.ip = NULL;
            free(dBowman.puerto);
            dBowman.puerto = NULL;
            
            close(fd);
        }
    }
    return 0;
}