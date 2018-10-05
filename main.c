#include <stdio.h>
#include "feeder.h"


int main(int argc, char* argv[]) {
    TQueue *url = malloc(sizeof(TQueue));
    TQueue *cert = malloc(sizeof(TQueue));
    int certFlag = 0;
    int tFlag = 0;
    int aFlag = 0;
    int uFlag = 0;
    QueueInit(url);
    QueueInit(cert);
    checkArg(argv,argc,url,cert,&certFlag,&tFlag,&aFlag,&uFlag);
    /*while(url->front != NULL){
        char *line;
        QueueFrontPop(url,&line);
        printf("%s",line);
        free(line);
    }
    printf("\n");
    while(cert->front != NULL){
        char *line;
        QueueFrontPop(cert,&line);
        printf("%s\n",line);
        free(line);
    }*/
    //printf("%d %d %d\n",tFlag,aFlag,uFlag);
    feedreader(url,cert,certFlag,tFlag,aFlag,uFlag);
    free(url);
    free(cert);
    return 0;
}