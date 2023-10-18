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

