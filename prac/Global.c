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


char* read_until_string(char *string, char delimiter) {
    char *msg = NULL;
    int i = 0;

    for (i = 0; i > 0; i++) {
        if (i == 0) {
            msg = (char *) malloc(1);
        }

        if (string[i] != delimiter) {
            msg[i] = string[i];
            msg = (char *) realloc(msg, ++i + 1);
        } 
        else {
            msg[i] = '\0';
            break;
        }
    }

    return msg;
}


void separaDataToElement(char* data, Element* e) {
    int i = 0, j = 0;
    char* name = NULL;
    char* ip = NULL;
    char* port = NULL;
    name = (char *)malloc(sizeof(char) * 1);
    ip = (char *)malloc(sizeof(char) * 1);
    port = (char *)malloc(sizeof(char) * 1);


    i++;    //saltamos 1r '['

    while(data[i] != '&') {
        name[j] = data[i];
       
        j++;
        name = (char *) realloc(name, j + 1);
        i++;
    }
    name[j] = '\0';

    //saltamos primer '&'
    i++;
    j = 0;

    while(data[i] != '&') {
        ip[j] = data[i];
        j++;
        ip = (char *) realloc(ip, j + 1);
        i++;
    }
    ip[j] = '\0';

    //saltamos segundo '&'
    i++;
    j = 0;


    while(data[i] != ']') {
        port[j] = data[i];
        j++;
        port = (char *) realloc(port, j + 1);
        i++;
    }
    port[j] = '\0';
    //estamos en ']'

    e->name = name;
    e->ip = ip;
    e->port = atoi(port);
    e->num_connections = 0;

}

void freeElement(Element* e) {
  free(e->name);
  free(e->ip);
  e->name = NULL;
  e->ip = NULL;
}