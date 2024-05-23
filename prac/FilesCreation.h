/*
Autors:
    Alba Mayol Lozano -->alba.mayol
    Kevin Eljarrat Ohayon --> kevin.eljarrat
    LaSalle - Sistemes Operatius
Data creació: 23/05/24
Data última modificació: 23/05/24
*/

#ifndef _FILESCREATION_H_
#define _FILESCREATION_H_

#define _GNU_SOURCE

#include <string.h>
#include <strings.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>

void createDirectory(char* directory);
void createStatsFile(char* directory);

#endif