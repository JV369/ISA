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
    feedreader(url,cert,certFlag,tFlag,aFlag,uFlag);
    QueueDestroy(url);
    free(url);
    QueueDestroy(cert);
    free(cert);
    return 0;
}