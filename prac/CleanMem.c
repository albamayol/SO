/*
Autors:
    Alba Mayol Lozano -->alba.mayol
    Kevin Eljarrat Ohayon --> kevin.eljarrat
    LaSalle - Sistemes Operatius
Data creació: 23/05/24
Data última modificació: 23/05/24
*/

#include "CleanMem.h"

/*
@Finalitat: cancelar i matar els threads de Poole que ja hagin acabat les descarregues
@Paràmetres: ThreadPoole** thread: array de descarrgues de Poole; int numThreads: mida de l'array de threads de Poole (connexions de Bowman)
@Retorn: ---
*/
void cleanThreadsPoole(ThreadPoole** threads, int numThreads) {
    write(1, "\nYou are waiting for the completion of the sending of the songs in progress to exit the program.\n", strlen("\nYou are waiting for the completion of the sending of the songs in progress to exit the program.\n"));
    
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
        write(1, "\nYou are waiting for the completion of the downloads of the songs in progress to exit the program.\n", strlen("\nYou are waiting for the completion of the downloads of the songs in progress to exit the program.\n"));
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
