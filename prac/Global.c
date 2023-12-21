/*
Autores:
    Alba Mayol Lozano -->alba.mayol
    Kevin Eljarrat Ohayon --> kevin.eljarrat
*/

#include "Global.h"

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

void freePoolesArray(Element *array, int size) {
    for (int i = 0; i < size; ++i) {
        freeElement(&array[i]);
    }
    free(array);
}


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

void createDirectory(char* directory) {	
    char *command = NULL;

    asprintf(&command, "mkdir -p %s", directory);
    
    FILE* pipe = popen(command, "r");
    if (pipe == NULL) {
        perror("popen");
        //exit(EXIT_FAILURE);
    } else {
        pclose(pipe); // Close the pipe
    }
    
    freeString(&command);
    
}

Element pooleMinConnections(Element *poole_list, int poole_list_size) {
    Element e;
    e.name = NULL, e.ip = NULL;
    e.port = 0;
    e.num_connections = -1;
    int j = 0;

    int minConnections = INT_MAX;
    char *buffer = NULL;
    
    if (poole_list_size != 0) {
        for (int i = 0; i < poole_list_size; i++) {

            write(1, poole_list[i].name, strlen(poole_list[i].name));
            write(1, poole_list[i].ip, strlen(poole_list[i].ip));
            
            asprintf(&buffer, "\npuerto: %d conexiones: %d\n",poole_list[i].port, poole_list[i].num_connections);
            printF(buffer);
            freeString(&buffer);

            if (poole_list[i].num_connections <= minConnections) {
                e.name = strdup(poole_list[i].name);
                e.ip = strdup(poole_list[i].ip);
                e.port = poole_list[i].port;
                e.num_connections = poole_list[i].num_connections;
                minConnections = poole_list[i].num_connections;
                write(1, e.name, strlen(e.name));
                write(1, e.ip, strlen(e.ip));
                j = i;
            
                asprintf(&buffer, "\npuerto: %d conexiones: %d minConnections: %d\n",e.port, e.num_connections, minConnections);
                printF(buffer);
                freeString(&buffer);
            }
        }
        poole_list[j].num_connections++;
    } 
    
    return e;
}

int decreaseNumConnections(Element *poole_list, int poole_list_size, char* pooleName) {
    for (int i = 0; i < poole_list_size; i++) {
        if (strcmp(poole_list[i].name, pooleName) == 0) {
            poole_list[i].num_connections--;
            return 1;
        }
    }
    return 0; //no se ha encontrado ese poole en discovery
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


