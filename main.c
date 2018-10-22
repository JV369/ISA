/**
 * Main Feedreaderu
 * Autor: Jan Vávra
 * Login: xvavra20
 */
#include <stdio.h>
#include "connect.h"

/**
 * Hlavní vstup programu
 * @param argc počet argumenů
 * @param argv pole argumentů
 * @return 0 v případě bezchybného průchodu,
 * 3x při chybě spojení a přenosu dat,
 * 5x při chybě v xml parsovani
 */
int main(int argc, char* argv[]) {
    if(argc == 1){
        help();
        return 0;
    }

    //fronta adres - nacte se v checkArg
    TQueue *url = malloc(sizeof(TQueue));
    //certifikáty
    char *certFile = NULL;
    char *certAddr = NULL;
    //flagy
    int tFlag = 0;
    int aFlag = 0;
    int uFlag = 0;
    QueueInit(url);
    //kontrola argumentů
    int retVal = checkArg(argv,argc,url,&certFile,&certAddr,&tFlag,&aFlag,&uFlag);
    if(retVal) {
        if(certFile != NULL)
            free(certFile);
        if(certAddr != NULL)
            free(certAddr);
        QueueDestroy(url);
        free(url);
        if(retVal == -10)
            return 0;
        return 30;
    }
    retVal = feedreader(url,certFile,certAddr,tFlag,aFlag,uFlag);
    if(certFile != NULL)
        free(certFile);
    if(certAddr != NULL)
        free(certAddr);
    QueueDestroy(url);
    free(url);
    if(retVal)
        return retVal;
    return 0;
}