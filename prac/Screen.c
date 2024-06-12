/*
Autors:
    Alba Mayol Lozano -->alba.mayol
    Kevin Eljarrat Ohayon --> kevin.eljarrat
    LaSalle - Sistemes Operatius
Data creació: 23/05/24
Data última modificació: 23/05/24
*/

#include "Screen.h"

pthread_mutex_t mutexScreen;

/*
@Finalitat: Inicialitza el mutex per printar per pantalla
@Paràmetres: ---
@Retorn: ---
*/
void initScreen() {
    pthread_mutex_init(&mutexScreen, NULL);
}

/*
@Finalitat: printa per pantalla la string donada, fent servir el mutex
@Paràmetres: char* x: string que es vol printar per pantalla
@Retorn: ---
*/
void printF(char* x) {
    //lock
    pthread_mutex_lock(&mutexScreen);
    //print
    write(1, x, strlen(x));
    //unlock
    pthread_mutex_unlock(&mutexScreen);
}

/*
@Finalitat: Destrueix el mutex de pantalla
@Paràmetres: ---
@Retorn: ---
*/
void destroyMutexScreen() {
    pthread_mutex_destroy(&mutexScreen);
}

/*
@Finalitat: Printa la informació llegida de configBowman
@Paràmetres: ---
@Retorn: ---
*/
void printInfoFileBowman() {
    char* msgAuxiliar = NULL;
    write(1, "\nFile read correctly:\n", strlen("\nFile read correctly:\n"));
    asprintf(&msgAuxiliar, "User - %s\nDirectory - %s\nIP - %s\nPort - %s\n\n", clienteName, pathClienteFile, ip, puerto);
    write(1, msgAuxiliar, strlen(msgAuxiliar));
    free(msgAuxiliar);
    msgAuxiliar = NULL;
}

/*
@Finalitat: Printa la informació llegida del configPoole
@Paràmetres: ---
@Retorn: ---
*/
void printInfoFile() {
    char* msgAuxiliar = NULL;
    asprintf(&msgAuxiliar, "\nFile read correctly:\nServer - %s\nServer Directory - %s\nIP Discovery - %s\nPort Server - %s\nIP Server - %s\nPort Server - %s\n\n", serverName, pathServerFile, ipDiscovery, puertoDiscovery, ipServer, puertoServer);
    write(1, msgAuxiliar, strlen(msgAuxiliar));
    free(msgAuxiliar);
    msgAuxiliar = NULL;
}

/*
@Finalitat: Printa les cançons per pantalla.
@Paràmetres: int numCanciones: número de cançons; char ***canciones: llista de strings amb les cançons a imprimir.
@Retorn: ---
*/
void printarSongs(int numCanciones, char ***canciones) {
    char* msgAuxiliar = NULL;
    asprintf(&msgAuxiliar, "\nThere are %d songs available for download:", numCanciones);
    write(1, msgAuxiliar, strlen(msgAuxiliar));
    free(msgAuxiliar);
    msgAuxiliar = NULL;

    for (int i = 0; i < numCanciones; i++) {
        if (i == numCanciones - 1) {
            asprintf(&msgAuxiliar, "\n%d. %s\n\n", i + 1, (*canciones)[i]);
        } else {
            asprintf(&msgAuxiliar, "\n%d. %s", i + 1, (*canciones)[i]);
        }

        write(1, msgAuxiliar, strlen(msgAuxiliar));
        free(msgAuxiliar);
        msgAuxiliar = NULL;
        free((*canciones)[i]);
    }
}

/*
@Finalitat: Printa la llista de playlists de Poole
@Paràmetres: int numListas: número de playlists que té Poole; char*** listas: llista de les playlists on cada playlist conté una llista de les seves cançons; int* numCancionesPorLista: número de cançons que té cada playlist
@Retorn: ---
*/
void printarPlaylists(int numListas, char ***listas, int *numCancionesPorLista, InfoPlaylist **arrayInfo, int *numInfoPlaylists) {
    char* msgAuxiliar = NULL;
    asprintf(&msgAuxiliar, "\nThere are %d lists available for download:", numListas);
    write(1, msgAuxiliar, strlen(msgAuxiliar));
    free(msgAuxiliar);
    msgAuxiliar = NULL;
    
    for (int i = 0; i < numListas; i++) {
        asprintf(&msgAuxiliar, "\n%d. %s", i + 1, listas[i][0]);
         write(1, msgAuxiliar, strlen(msgAuxiliar));
        free(msgAuxiliar);
        msgAuxiliar = NULL;

        // Guardamos info Playlists actuales
        *arrayInfo = realloc(*arrayInfo, sizeof(InfoPlaylist) * ((*numInfoPlaylists) + 1));
        (*arrayInfo)[*numInfoPlaylists].nameplaylist = strdup(listas[i][0]); 
        (*arrayInfo)[*numInfoPlaylists].numSongs = numCancionesPorLista[i];
        *numInfoPlaylists = *numInfoPlaylists + 1;

        free(listas[i][0]);
        
        for (int j = 1; j <= numCancionesPorLista[i]; j++) {
            asprintf(&msgAuxiliar, "\n   %c. %s", 'a' + j - 1, listas[i][j]);
             write(1, msgAuxiliar, strlen(msgAuxiliar));
            free(msgAuxiliar);
            msgAuxiliar = NULL;
            free(listas[i][j]);
        }

        free(listas[i]); 
    }
    write(1, "\n\n", strlen("\n\n"));
    

    free(listas); 
    free(numCancionesPorLista);
}
