/**
 * Hlavičkový soubor - Zpracování požadavků na server
 * Autor: Jan Vávra
 * Login: xvavra20
 */

#include <stdio.h>
#include <openssl/ssl.h>
#include <openssl/bio.h>
#include <openssl/err.h>
#include <openssl/evp.h>
#include <openssl/conf.h>
#include "feeder.h"

/**
 * Vstupní bod
 * @param url - fronta adres
 * @param certFile - adresa k souboru s certifikátem
 * @param certAddr - adresa k adresáři s certifikáty
 * @param tFlag - flag čas
 * @param aFlag - flag autor
 * @param uFlag - flag url
 * @return 0 pokud je vše ok,
 *         31 při chybě v této funkci,
 *         32 při pokusu se připojit,
 *         33 při zpracování hlavičky a xml,
 *         5x při chybě v parseru
 */
int feedreader(TQueue *url, char *certFile, char *certAddr, int tFlag, int aFlag, int uFlag);