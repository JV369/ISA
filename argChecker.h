/**
 * Hlavičkový soubor pro frontu
 * Autor: Jan Vávra
 * Login: xvavra20
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <dirent.h>
#include "queue.h"
#include <regex.h>

int checkArg(char **arguments,int lenght, TQueue *url, char *certFile,char *certAddr, int *tFlag, int *aFlag, int *uFlag);
