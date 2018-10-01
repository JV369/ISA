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
        printf("%s",line);
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
int checkArg(char **arguments,int lenght, TQueue *url, TQueue *cert){
    int urlFlag = 0;
    int certFlag = 0;
    int result = 0;

    for (int i = 1; i < lenght; i++) {
        if(strcmp(arguments[i],"-f") == 0){
            urlFlag = loadUrlFile(url,arguments[i+1]);
            urlFlag = 1;
            i++;
        }
        else if(strcmp(arguments[i],"-c") == 0){
            QueueUp(url,arguments[i+1]);
            certFlag = 1;
            i++;
        }
        else if(strcmp(arguments[i],"-C") == 0){
            int j = 0;
            while(arguments[i+1][j] != '\0')
                j++;
            if(arguments[i+1][j-1] != '/'){
                char *str = (char *)malloc(strlen(arguments[i+1])+2);
                strcpy(str,arguments[i+1]);
                strcat(str,"/");
                loadCertDir(cert,str);
                free(str);
            }
            else
                loadCertDir(cert,arguments[i+1]);
            certFlag = 1;
            i++;
        }
        else if(strcmp(arguments[i],"-T") == 0){

        }
        else if(strcmp(arguments[i],"-a") == 0){

        }
        else if(strcmp(arguments[i],"-u") == 0){

        }
        else{
            QueueUp(url,arguments[i]);
            urlFlag = 1;
        }
    }
    if(!urlFlag){
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
