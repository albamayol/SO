/*
Autores:
    Alba Mayol Lozano -->alba.mayol
    Kevin Eljarrat Ohayon --> kevin.eljarrat
*/

#include "Global.h"

char* read_until(int fd, char delimiter) {
    char *msg = NULL;
    char current;
    int i = 0;

    while(read(fd, &current, 1) > 0){
        if (i == 0) {
            msg = (char *) malloc(1);
        }

        if (current != delimiter) {
            msg[i] = current;
            msg = (char *) realloc(msg, ++i + 1);
        } 
        else {
            msg[i] = '\0';
            break;
        }
    }  

    return msg;
}

Element pooleMinConnections(LinkedList * list) {
    Element e ;
    e.name = NULL;
    e.ip = NULL;
    e.port = 0;
    e.num_connections = -1;
    
    int minConnections = 0;

    if (!LINKEDLIST_isEmpty(*list)) {
        LINKEDLIST_goToHead (list);
        while(!LINKEDLIST_isAtEnd(*list)) {
            Element e_aux = LINKEDLIST_get(list);
            if (e_aux.num_connections <= minConnections) {
                e.name = strdup(e_aux.name);
                e.ip = strdup(e_aux.ip);
                e.port = e_aux.port;
                e.num_connections = e_aux.num_connections;
            }
            freeElement(&e_aux);
            LINKEDLIST_next(list);
        }
        
    } 
    return e;
}

