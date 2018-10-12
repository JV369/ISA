#include <stdio.h>
#include "connect.h"


int main(int argc, char* argv[]) {
    TQueue *url = malloc(sizeof(TQueue));
    char *certFile = NULL;
    char *certAddr = NULL;
    int tFlag = 0;
    int aFlag = 0;
    int uFlag = 0;
    QueueInit(url);
    if(checkArg(argv,argc,url,certFile,certAddr,&tFlag,&aFlag,&uFlag)) {
        QueueDestroy(url);
        free(url);
        return 30;
    }
    int retVal = feedreader(url,certFile,certAddr,tFlag,aFlag,uFlag);
    if(retVal) {
        QueueDestroy(url);
        free(url);
        return retVal;
    }
    QueueDestroy(url);
    free(url);
    return 0;
}