//
// Created by jan on 29.9.18.
//

#include "argChecker.h"

regex_t regex;

int loadUrlFile(TQueue *url, char *file){
    FILE *fp;
    fp = fopen(file,"r");
    if(fp == NULL) {
        fprintf(stderr,"Chyba pri otevreni souboru\n");
        return -1;
    }

    size_t size = 0;
    char *line = NULL;
    while((getline(&line,&size,fp)) != -1){
        if(line[0] == '#'){
            continue;
        }
        char *token = strtok(line,"\n");
        if(regexec(&regex,line,0,NULL,0)) {
            continue;
        }
        QueueUp(url,token);
    }

    if(line != NULL)
        free(line);
    fclose(fp);
    return 0;
}

/**
 * vlastní getopt
 */
int checkArg(char **arguments,int lenght, TQueue *url, char *certFile,char *certAddr, int *tFlag, int *aFlag, int *uFlag){
    int urlFlag = 0;
    int urlFFlag = 0;
    int position = 1;
    regcomp(&regex, "^https?://.+(:[0-9]+)?/.*",REG_EXTENDED);
    for (int i = 1; i < lenght; i++) {
        if (arguments[i][0] == '-') {
            int ops = 1;
            int skip = 0;
            for (int k = 1; k < strlen(arguments[i]); k++) {
                if (arguments[i][k] == 'f') {
                    loadUrlFile(url, arguments[i + ops]);
                    if(url->front != NULL)
                        urlFFlag = 1;
                    ops++;
                    skip++;
                } else if (arguments[i][k] == 'c'){
                    certFile = (char *)malloc(strlen(arguments[i + ops])+1);
                    strcpy(certFile,arguments[i + ops]);
                    skip++;
                    ops++;
                } else if (arguments[i][k] == 'C') {
                    certAddr = (char *)malloc(strlen(arguments[i + ops])+1);
                    strcpy(certAddr,arguments[i + ops]);
                    skip++;
                    ops++;
                } else if (arguments[i][k] == 'T') {
                    *tFlag = position;
                    position++;
                } else if (arguments[i][k] == 'a') {
                    *aFlag = position;
                    position++;
                } else if (arguments[i][k] == 'u') {
                    *uFlag = position;
                    position++;
                } else{
                    fprintf(stderr,"Chyba spatny prepinac");
                    return -1;
                }
            }
            i += skip;
        }
        else if(!urlFlag){
            //printf("%s\n",arguments[i]);
            if(regexec(&regex,arguments[i],0,NULL,0)) {
                fprintf(stderr,"Chyba: nesprávný formát adresy");
                continue;
            }
            QueueUp(url, arguments[i]);
            urlFlag = 1;
        }
    }
    if(!urlFlag && !urlFFlag){
        char *line = NULL;
        size_t size = 0;
        printf("Nezadal jste platnou adresu pro cteni. Prosim zadejte adresu.\n");
        getline(&line,&size,stdin);
        QueueUp(url,line);
        if(line != NULL)
            free(line);
    }
    regfree(&regex);
    return 0;
}
