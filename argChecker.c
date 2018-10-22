/**
 * Kód pro vlastní validátor argumenů
 * Autor: Jan Vávra
 * Login: xvavra20
 */

#include "argChecker.h"

//regex pro kontrolu url
regex_t regex;

/**
 * Načte url ze souboru do fronty
 * @param url - fronta s url adresami
 * @param file - soubor ze kterého čteme
 * @return 0 pokud nenastala chyba, jinak 1
 */
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
        //ignorování zakomentovaných řádků
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

void help(){
    printf("feedreader <URL | -f <feedfile>> [-c <certfile>] [-C <certaddr>] [-T] [-a] [-u]\n\n");
    printf("URL -- zdroj feedu\n");
    printf("-f <feedfile> -- přepínač pro zadání souboru se zdroji feedů (feedfile)\n");
    printf("*** URL nebo -f <feedfile> je třeba zadat na vstupu ***\n\n");
    printf("-c <certfile> -- (volitelný) přepínač pro zadání souboru s certifikátem pro připojení (certfile)\n");
    printf("-C <certaddr> -- (volitelný) přepínač pro zadání adresáře s certifikáty pro připojení (certaddr)\n\n");
    printf("-T -- (volitelný) přepínač pro vypsání času aktualizace položek feedu\n");
    printf("-a -- (volitelný) přepínač pro vypsání autorů položek feedu\n");
    printf("-u -- (volitelný) přepínač pro vypsání URL položek feedu\n");
    return;
}

/**
 * Vlastní kontrola argumenů
 * @param arguments - pole argumenů
 * @param lenght - delka pole argumenů
 * @param url - fronta url adres (prazdná při zavolání)
 * @param certFile - cesta k souboru s certifikátem
 * @param certAddr - cesta k adresáři s certifikátama
 * @param tFlag - flag pro čas ve feedu
 * @param aFlag - flag pro autora ve feedu
 * @param uFlag - flag pro url ve feedu
 * @return 0 v přidadě správnosti a počtu argumenů, jinak -1
 */
int checkArg(char **arguments,int lenght, TQueue *url, char **certFile,char **certAddr, int *tFlag, int *aFlag, int *uFlag){
    int urlFlag = 0;
    int urlFFlag = 0;
    int position = 1;
    //regex prom kontrolu url adresy
    regcomp(&regex, "^https?://.+(:[0-9]+)?/.*",REG_EXTENDED);
    for (int i = 1; i < lenght; i++) {
        //ocekavam flagy
        if (arguments[i][0] == '-') {
            int ops = 1;
            int skip = 0;
            for (int k = 1; k < strlen(arguments[i]); k++) {
                //file flag - nactu url ze souboru
                if (arguments[i][k] == 'f') {
                    loadUrlFile(url, arguments[i + ops]);
                    if(url->front != NULL)
                        urlFFlag = 1;
                    ops++;
                    skip++;
                }
                //cert file - načtu cestu k certifikátu
                else if (arguments[i][k] == 'c'){
                    *certFile = (char *)malloc(strlen(arguments[i + ops])+1);
                    strcpy(*certFile,arguments[i + ops]);
                    skip++;
                    ops++;
                }
                //cert addr - načtu cestu k certifikátům
                else if (arguments[i][k] == 'C') {
                    *certAddr = (char *)malloc(strlen(arguments[i + ops])+1);
                    strcpy(*certAddr,arguments[i + ops]);
                    skip++;
                    ops++;
                }
                //time flag
                else if (arguments[i][k] == 'T') {
                    *tFlag = position;
                    position++;
                }
                //author flag
                else if (arguments[i][k] == 'a') {
                    *aFlag = position;
                    position++;
                }
                //url flag
                else if (arguments[i][k] == 'u') {
                    *uFlag = position;
                    position++;
                }
                else if (arguments[i][k] == 'h') {
                    help();
                    regfree(&regex);
                    return -10;
                }
                //jakýkoli jiný flag je špatný -> chyba
                else{
                    fprintf(stderr,"Chyba spatny prepinac");
                    help();
                    return -1;
                }
            }
            i += skip;
        }
        else if(!urlFlag){
            //printf("%s\n",arguments[i]);
            if(regexec(&regex,arguments[i],0,NULL,0)) {
                fprintf(stderr,"Chyba: nesprávný formát adresy");
                help();
                continue;
            }
            QueueUp(url, arguments[i]);
            urlFlag = 1;
        }
    }
    regfree(&regex);
    if(!urlFlag && !urlFFlag){
        help();
        return -1;
    }
    return 0;
}
