/*
Autors:
    Alba Mayol Lozano -->alba.mayol
    Kevin Eljarrat Ohayon --> kevin.eljarrat
    LaSalle - Sistemes Operatius
Data creació: 17/10/23
Data última modificació: 16/5/24
*/

#include "Global.h"

/*
@Finalitat: Alliberar la memòra d'un Element de la llista de Poole's de Discovery
@Paràmetres: Element* e: Element a alliberar
@Retorn: ---
*/
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



/*
@Finalitat: Allibera cada Element de l'array de Poole's de Discovery al igual que el propi array també
@Paràmetres: Element* array: array de Poole's de Discovery
@Retorn: ---
*/
void freePoolesArray(Element *array, int size) {
    for (int i = 0; i < size; ++i) {
        freeElement(&array[i]);
    }
    free(array);
}



/*
@Finalitat: Retorna el més petit de 2 valors donats
@Paràmetres: size_t a: primer valor; size_t b: segon valor
@Retorn: int: valor més petit
*/
int min(size_t a, size_t b) {
    if (a < b) {
        return a;
    }
    return b;
}

/*
@Finalitat: Genera un número aleatori
@Paràmetres: ---
@Retorn: int: número aleatori
*/
int getRandomID() { 
    return (rand() % 999) + 1; 
}

/*
@Finalitat: Busca el Poole amb el mínim de connexions dins els que hi ha a l'array de Poole's de Discovery
@Paràmetres: Element* poole_list: array de Poole's de Discovery; int* poole_list_size: mida de l'array de Poole's
@Retorn: Element: El Poole amb el mínim de connexions
*/
Element pooleMinConnections(Element *poole_list, int poole_list_size) {
    Element e;
    e.name = NULL, e.ip = NULL;
    e.port = 0;
    e.num_connections = -1;
    int j = 0;

    int minConnections = INT_MAX;
    if (poole_list_size != 0) {
        for (int i = 0; i < poole_list_size; i++) {
            if (poole_list[i].num_connections <= minConnections) {
                e.name = strdup(poole_list[i].name);
                e.ip = strdup(poole_list[i].ip);
                e.port = poole_list[i].port;
                e.num_connections = poole_list[i].num_connections;
                minConnections = poole_list[i].num_connections;
                j = i;
            }
        }
        poole_list[j].num_connections++;
    } 
    
    return e;
}

/*
@Finalitat: Printa la llista de Poole's de Discovery, amb la informació rellevant de cada Poole
@Paràmetres: Element* poole_list: array de Poole's de Discovery; int* poole_list_size: mida de l'array de Poole's
@Retorn: ---
*/
void printListPooles(Element *poole_list, int poole_list_size) {
    char* buffer = NULL;
    if (poole_list_size != 0) {
        printf("\nLISTA ACTUAL DE POOLES:\n");
        for (int i = 0; i < poole_list_size; i++) {
            asprintf(&buffer, "\nnombre: %s\nip: %s\npuerto: %d\nconexiones: %d\n",poole_list[i].name, poole_list[i].ip, poole_list[i].port, poole_list[i].num_connections);
            printF(buffer);
            freeString(&buffer);
        }
    } 
}

/*
@Finalitat: Decrementa en 1 el número de connexions de Bowman's d'un Poole en concret dins la llista de Poole's de Discovery
@Paràmetres: Element *poole_list: llista de Poole's; int poole_list_size: mida de la llista de Poole's; char* pooleName: nom del Poole
@Retorn: int: 0 si no s'ha trobat el Poole, 1 si s'ha trobat i decrementat el num_connexions del Poole
*/
int decreaseNumConnections(Element *poole_list, int poole_list_size, char* pooleName) {
    for (int i = 0; i < poole_list_size; i++) {
        printF(poole_list[i].name);
        if (strcmp(poole_list[i].name, pooleName) == 0) {
            poole_list[i].num_connections--;
            return 1;
        }
    }
    return 0; 
}

/*
@Finalitat: Elimina un Poole de l'array de Poole de Discovery
@Paràmetres: Element** poole_list: array de Poole's de Discovery; int* poole_list_size: mida de l'array de Poole's; char* pooleName: nom del Poole a esborrar
@Retorn: int: 0 si no s'ha trobat aquell Poole, 1 si s'ha trobat i esborrat aquell Poole
*/
int erasePooleFromList(Element** poole_list, int* poole_list_size, char* pooleName) {
    int flagFound = 0;
    Element* updatedPooleList = NULL;
    int updatedPooleListSize = 0;

    for (int i = 0; i < *poole_list_size; i++) {
        if (strcmp((*poole_list)[i].name, pooleName) != 0) {
            updatedPooleList = realloc(updatedPooleList, sizeof(Element) * (updatedPooleListSize + 1));
            updatedPooleList[updatedPooleListSize].name = strdup((*poole_list)[i].name);
            updatedPooleList[updatedPooleListSize].ip = strdup((*poole_list)[i].ip);
            updatedPooleList[updatedPooleListSize].port = (*poole_list)[i].port;
            updatedPooleList[updatedPooleListSize].num_connections = (*poole_list)[i].num_connections;
            updatedPooleListSize++;
        } else {
            flagFound = 1;
        }
    }
    freePoolesArray(*poole_list, *poole_list_size);
    *poole_list_size = updatedPooleListSize;
    *poole_list = updatedPooleList;
    
    return flagFound; 
}

/*
@Finalitat: Separa el camp data en les diferents informacions del nou Poole conectat a Discovery
@Paràmetres: char* data: camp data, Element* e: estructura de dades de Poole
@Retorn: ---
*/
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


    while(data[i] != '\0') {
        port[j] = data[i];
        j++;
        port = (char *) realloc(port, j + 1);
        i++;
    }
    port[j] = '\0';

    e->name = strdup(name);
    e->ip = strdup(ip);
    e->port = atoi(port);
    e->num_connections = 0;

    freeString(&name);
    freeString(&ip);
    freeString(&port);
}