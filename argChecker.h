/**
 * Hlavičkový soubor vlastní validátor argumenů
 * Autor: Jan Vávra
 * Login: xvavra20
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <dirent.h>
#include "queue.h"
#include <regex.h>

/**
 * Vypsání pomoci pro program
 */
void help();
/**
 * Vlastní kontrola argumenů
 * @param arguments - pole argumenů
 * @param lenght - delka pole argumenů
 * @param url - fronta url adres (prazdná při zavolání)
 * @param certFile - cesta k souboru s certifikátem
 * @param certAddr - cesta k adresáři s certifikátama
 * @param tFlag - flag pro čas ve feedu
 * @param aFlag - flag pro autora ve feedu
 * @param uFlag - flag pro url ve feedu
 * @return 0 v přidadě správnosti a počtu argumenů, jinak -1
 */
int checkArg(char **arguments,int lenght, TQueue *url, char **certFile,char **certAddr, int *tFlag, int *aFlag, int *uFlag);
