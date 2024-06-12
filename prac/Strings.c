/*
Autors:
    Alba Mayol Lozano -->alba.mayol
    Kevin Eljarrat Ohayon --> kevin.eljarrat
    LaSalle - Sistemes Operatius
Data creació: 23/05/24
Data última modificació: 23/05/24
*/

#include "Strings.h"

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
@Finalitat: Processa les trames de cançons per separar-les en una llista.
@Paràmetres: char ***canciones: array de strings on es guarden les cançons; char *songs: string amb les cançons concatenades.
@Retorn: El número de cançons processades.
*/
int procesarTramasSongs(char ***canciones, char *songs) {
    int numCanciones = 0, inicio = 0;
    char final = ' ', *song = NULL;

    do {
        song = readUntilFromIndex(songs, &inicio, '&', &final, '\0');

        *canciones = realloc(*canciones, (numCanciones + 1) * sizeof(char *));
        if (*canciones == NULL) {
            break;
        }
        (*canciones)[numCanciones] = song;
        numCanciones++;
    } while (final != '\0');
    return numCanciones;
}

/*
@Finalitat: Processa les trames de cançons i les guarda a una llista de strings.
@Paràmetres: char ***canciones: array de strings on es guardaran les cançons processades; char *songs: string amb les cançons concatenades.
@Retorn: Nombre de cançons processades.
*/
char ***procesarTramasPlaylists(char *playlists, int **numCancionesPorLista, int numCanciones, int *numListas) {
    int i = 0, totalCanciones = 0, inicioPlaylist = 0, inicioSong = 0;
    char valorFinal = ' ', ***listas = NULL, *playlist = NULL, *song = NULL;
    do {
        playlist = readUntilFromIndex(playlists, &inicioPlaylist, '#', &valorFinal, '\0');

        size_t len = strlen(playlist);
        playlist = realloc(playlist, len + 2);
        if (playlist == NULL) {
            break;
        }
        playlist[len] = '#';
        playlist[len + 1] = '\0';

        inicioSong = 0;
        i = 0;
        valorFinal = ' ';
        do {    
            song = readUntilFromIndex(playlist, &inicioSong, '&', &valorFinal, '#');
            if (i == 0) {
                // Primero, reservamos memoria para almacenar una nueva lista
                listas = realloc(listas, ((*numListas) + 1) * sizeof(char **));
                if (listas == NULL) {
                    break;
                }

                listas[*numListas] = malloc(sizeof(char *));
                if (listas[*numListas] == NULL) {
                    break;
                }
                
                listas[*numListas][i] = strdup(song); 
                if (listas[*numListas][i] == NULL) {
                    free(listas[*numListas]);
                    break;
                }
            } else {
                listas[*numListas] = realloc(listas[*numListas], (i + 1) * sizeof(char *));
                if (listas[*numListas] == NULL) {
                    break;
                }

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
            freeString(&song);
        } while (valorFinal != '#');
        totalCanciones += (*numCancionesPorLista)[*numListas];
        (*numListas)++;

        (*numCancionesPorLista) = realloc((*numCancionesPorLista), (*numListas + 1) * sizeof(int));
        (*numCancionesPorLista)[(*numListas)] = 0;
        if (*numCancionesPorLista == NULL) {
            break;
        }
        freeString(&playlist);
    } while (totalCanciones < numCanciones);

    return listas;
}