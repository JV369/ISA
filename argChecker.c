//
// Created by jan on 29.9.18.
//

#include "argChecker.h"

int loadUrlFile(TQueue *url, char *file){
    FILE *fp;
    fp = fopen(file,"r");
    if(fp == NULL) {
        fprintf(stderr,"Chyba pri otevreni souboru\n");
        return -1;
    }

    size_t size = 0;
    char *line = NULL;
    while(getline(&line,&size,fp) != -1){
        if(line[0] == '#'){
            continue;
        }
        QueueUp(url,line);
    }

    if(line != NULL)
        free(line);
    fclose(fp);
    return 0;
}

int loadCertDir(TQueue *cert,char *dir){
    struct dirent *dirIn;
    DIR *dp;
    dp = opendir(dir);
    if(dp == NULL){
        fprintf(stderr, "Chyba pri otevirani adresare s certifikatama\n");
    }

    while ((dirIn = readdir(dp)) != NULL){
        char *str = (char *)malloc(strlen(dir)+strlen(dirIn->d_name)+1);
        strcpy(str,dir);
        strcat(str,dirIn->d_name);
        QueueUp(cert,str);
        free(str);
    }
    closedir(dp);
    return 0;

}

/**
 * vlastní getopt
 */
int checkArg(char **arguments,int lenght, TQueue *url, TQueue *cert, int *certFlag, int *tFlag, int *aFlag, int *uFlag){
    int urlFlag = 0;
    int urlFFlag = 0;
    for (int i = 1; i < lenght; i++) {
        if (arguments[i][0] == '-') {
            int ops = 1;
            int skip = 0;
            for (int k = 1; k < strlen(arguments[i]); k++) {
                if (arguments[i][k] == 'f') {
                    loadUrlFile(url, arguments[i + ops]);
                    urlFFlag = 1;
                    ops++;
                    skip++;
                } else if (arguments[i][k] == 'c'){
                    QueueUp(url, arguments[i + 1]);
                    *certFlag = 1;
                    skip++;
                    ops++;
                } else if (arguments[i][k] == 'C') {
                    int j = 0;
                    while (arguments[i + 1][j] != '\0')
                        j++;
                    if (arguments[i + 1][j - 1] != '/') {
                        char *str = (char *) malloc(strlen(arguments[i + 1]) + 2);
                        strcpy(str, arguments[i + 1]);
                        strcat(str, "/");
                        loadCertDir(cert, str);
                        free(str);
                    } else
                        loadCertDir(cert, arguments[i + 1]);
                    *certFlag = 1;
                    skip++;
                    ops++;
                } else if (arguments[i][k] == 'T') {
                    *tFlag = 1;
                } else if (arguments[i][k] == 'a') {
                    *aFlag = 1;
                } else if (arguments[i][k] == 'u') {
                    *uFlag = 1;
                } else{
                    fprintf(stderr,"Chyba spatny prepinac");
                    return -1;
                }
            }
            i += skip;
        }
        else if(!urlFlag){
            QueueUp(url, arguments[i]);
            urlFlag = 1;
        }
    }
    if(!urlFlag && !urlFFlag){
        char *line = NULL;
        size_t size = 0;
        printf("Nezadal jste adresu pro cteni. Prosim zadejte adresu.\n");
        getline(&line,&size,stdin);
        QueueUp(url,line);
        if(line != NULL)
            free(line);
    }
    return 0;
}
