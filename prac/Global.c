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

Element pooleMinConnections(LinkedList list) {
    Element e;
    e.name = NULL, e.ip = NULL;
    e.port = 0;
    e.num_connections = -1;
    
    int minConnections = 0;

    if (!LINKEDLIST_isEmpty(list)) {
        LINKEDLIST_goToHead (&list);
        while(!LINKEDLIST_isAtEnd(list)) {
            Element e_aux = LINKEDLIST_get(&list);
            if (e_aux.num_connections <= minConnections) {
                e.name = strdup(e_aux.name);
                e.ip = strdup(e_aux.ip);
                e.port = e_aux.port;
                e.num_connections = e_aux.num_connections;
            }
            freeElement(&e_aux);
            LINKEDLIST_next(&list);
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

char* convertIntToString(int num) {
    int numDigits = snprintf(NULL, 0, "%d", num);  //calcula el numero de digitos necesarios para interpretar un int como string
    char* string = (char*)malloc(sizeof(char)*(numDigits + 1)); 
    sprintf(string, "%d", num); //CONVERSION --> sprintf está diseñada para formatear y printar uns string. Añade automaticamente el \0 al final. Si no hubiese hecho malloc +1 para el \0, el último caracter se lo hubiese "comido" el \0

    return string;
}

void separaDataToElement(char* data, Element* e) {
    int i = 0, j = 0;
    char *name = NULL, *ip = NULL, *port = NULL; 

    name = (char *)malloc(sizeof(char));
    ip = (char *)malloc(sizeof(char));
    port = (char *)malloc(sizeof(char));

    while(data[i] != '&') {
        name[j] = data[i];
        j++;
        name = (char *) realloc(name, j + 1);
        i++;
    }
    name[j] = '\0';
    write(1, "\n", 1);
    write(1, name, strlen(name));

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
    write(1, "\n", 1);
    write(1, ip, strlen(ip));
    //saltamos segundo '&'
    i++;
    j = 0;


    while(data[i] != '\0') {
        port[j] = data[i];
        j++;
        port = (char *) realloc(port, j + 1);
        i++;
    }
    port[j] = '\0';
    write(1, "\n", 1);
    write(1, port, strlen(port));
    write(1, "\n", 1);

    e->name = strdup(name);
    e->ip = strdup(ip);
    e->port = atoi(port);
    e->num_connections = 0;

    freeString(&name);
    freeString(&ip);
    freeString(&port);
}


void freeElement(Element* e) {
    if (e->name != NULL) {
        free(e->name);
        e->name = NULL;
    }

    if (e->ip != NULL) {
        free(e->ip);
        e->ip = NULL;
    }
}

void freeString(char **string) {
    if (*string != NULL) {
        free(*string);
        *string = NULL;
    }
}