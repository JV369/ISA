#include <stdio.h>
#include "argChecker.h"


int main(int argc, char* argv[]) {
    TQueue *url = malloc(sizeof(TQueue));
    TQueue *cert = malloc(sizeof(TQueue));
    QueueInit(url);
    QueueInit(cert);
    checkArg(argv,argc,url,cert);
    //printf("\n");
    while(url->front != NULL){
        char *line;
        QueueFrontPop(url,&line);
        printf("%s",line);
        free(line);
    }
    while(cert->front != NULL){
        char *line;
        QueueFrontPop(cert,&line);
        printf("%s\n",line);
        free(line);
    }
    free(url);
    free(cert);
    return 0;
}