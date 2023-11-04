/*
Autores:
    Alba Mayol Lozano -->alba.mayol
    Kevin Eljarrat Ohayon --> kevin.eljarrat
*/

#include "Global.h"

dataDiscovery dDiscovery;

/*
@Finalitat: Inicializar las variables a NULL.
@Paràmetres: ---
@Retorn: ---
*/
void inicializarDataDiscovery() {
    dDiscovery.ipPoole = NULL;
	dDiscovery.portPoole = NULL;
    dDiscovery.ipBowman = NULL; 
    dDiscovery.portBowman = NULL;
}

/*
@Finalitat: Manejar la recepción de la signal (SIGINT) y liberar los recursos utilizados hasta el momento.
@Paràmetres: ---
@Retorn: ---
*/
void sig_func() {
    if (dDiscovery.ipPoole != NULL) {
        free(dDiscovery.ipPoole);
        dDiscovery.ipPoole = NULL;
    }
    if (dDiscovery.portPoole != NULL) {
        free(dDiscovery.portPoole);
        dDiscovery.portPoole = NULL;
    }
    if (dDiscovery.ipBowman != NULL) {
        free(dDiscovery.ipBowman);
        dDiscovery.ipBowman = NULL;
    }
    if (dDiscovery.portBowman != NULL) {
        free(dDiscovery.portBowman);
        dDiscovery.portBowman = NULL;
    }
    exit(EXIT_FAILURE);
}

/*
@Finalitat: Implementar el main del programa.
@Paràmetres: ---
@Retorn: int: Devuelve 0 en caso de que el programa haya finalizado exitosamente.
*/
int main(int argc, char ** argv) {
    
    inicializarDataDiscovery();
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
            dDiscovery.ipPoole = read_until(fd, '\n');
            dDiscovery.portPooleString = read_until(fd, '\n');
            dDiscovery.portPoole = atoi(dDiscovery.portPooleString);
            dDiscovery.ipBowman = read_until(fd, '\n');
            dDiscovery.portBowmanString = read_until(fd, '\n');
            dDiscovery.portBowman = atoi(dDiscovery.portBowmanString);

            // Create the socket for the poole
            dDiscovery.fdPoole = socket (AF_INET, SOCK_STREAM, IPPROTO_TCP);
            if (dDiscovery.fdPoole < 0) {
                perror ("socket TCP");
                exit (EXIT_FAILURE);
            }

            // Specify the adress and port of the remote host
            bzero (&dDiscovery.poole_addr, sizeof (dDiscovery.poole_addr));
            dDiscovery.poole_addr.sin_family = AF_INET;
            dDiscovery.poole_addr.sin_port = htons (dDiscovery.portPoole);
            dDiscovery.poole_addr.sin_addr = dDiscovery.ipPoole;


            

            free(dDiscovery.ipPoole);
            dDiscovery.ipPoole = NULL;
            free(dDiscovery.portPoole);
            dDiscovery.portPoole = NULL;
            free(dDiscovery.ipBowman);
            dDiscovery.ipBowman = NULL;
            free(dDiscovery.portBowman);
            dDiscovery.portBowman = NULL;

            close(fd);
        }
    }
    return 0;
}