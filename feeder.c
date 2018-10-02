#include "feeder.h"


int getNoSslFeed(char *url, char **output){

}

int getSslFeed(char *url, TQueue *cert, int certFlag, char **output){

}

int feedreader(TQueue *url, TQueue *cert, int certFlag, int tFlag, int aFlag, int uFlag){

    while(url->front != NULL){
        int ssl = 0;
        char *activeUrl = NULL;
        QueueFrontPop(url,&activeUrl);
        char *token = strtok(activeUrl,"/");
        if(strcmp(token,"https:") == 0){
            ssl = 1;
        }
        else if (strcmp(token,"http:") != 0){
            //je mozne zadat bez http na zacatku?
            fprintf(stderr,"Chyba spatna url");
            return -1;
        }
        
    }
    return 0;
}