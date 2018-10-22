/**
 * Kód - Zpracování požadavků na server
 * Autor: Jan Vávra
 * Login: xvavra20
 */

#include "connect.h"

//proměnné pro kontrolování hlavičky
//v případě nenačtení celé hlavičky potřebuju hodnoty uchovat
//pass - sedí Content-Type - čekám xml soubor
int pass = 0;
//newLine - načetli jsme znak nového řádku - možný konec hlavičky
int newLine = 0;
//ok - vrátil se nám 200 OK
int ok = 0;

/**
 * Kontrola hlavičky
 * @param input vstupní řetězec
 * @param chunk flag, pokud je zapnuto chunkování, vrátí 1 (výchozí hodnota 0)
 * @return počet znaků pro přeskočení hlavičky,
 * v případě chyby -2,
 * v případě nenačtení celé hlavičky -1
 */
int analyseHeader(char *input,int *chunk){
    //printf("%s\n\n",input);
    //printf("%d %d\n",ok,pass);
    char *line = calloc(1024,sizeof(char *));
    regex_t contentType;
    regcomp(&contentType,"^Content-Type: .*/.*xml.*",REG_EXTENDED);
    int j = 0;
    for (int i = 0;; i++) {
        //narazili jsme na konec řádku
        if(input[i] == '\r' || input[i] == '\n'){
            //2 znaky konce řádku po sobě znamenají konec hlavičky(formát \r\n\r\n nebo \n\n)
            if(newLine) {
                i+=2;
                free(line);
                regfree(&contentType);
                //printf("%d %d\n",ok,pass);
                if(pass)
                    return i;
                return -2;
            }
            //první řádek v hlavičce je návratový kód - zkontrolujeme
            else if(!ok){
                if(strcmp(line,"HTTP/1.1 200 OK") != 0){
                    free(line);
                    regfree(&contentType);
                    return -2;
                }
                ok = 1;
            }
            //zapnuto chunkování
            else if(strcmp(line,"Transfer-Encoding: chunked") == 0)
                *chunk = 1;
            //vrátil se nám xml soubor, nastavíme flag pass
            else if(!regexec(&contentType,line,0,NULL,0))
                pass = 1;
            //pokud je to poprvé co jsme narazili na \n, nastavíme flag newLine
            else if(input[i] == '\n'){
                newLine = 1;
                continue;
            }
            j = 0;
            //printf("%s\n",line);
            memset(line,0,128);
            continue;
        }
        //konec řetězce, hlavička není načtená celá
        else if(!(input[i])){
            free(line);
            regfree(&contentType);
            if(!ok)
                return -2;
            return -1;
        }

        line[j] = input[i];
        j++;
        newLine = 0;
    }
}

/**
 * Zbaví výsledné xml chunků
 * @param io vstupní/výstupní řetězec
 * @return 0 pokud je vše v pořádku , v případě chyby 1
 */
int uniteChunks(char **io){
    //printf("%s\n",*io);
    char chunkLine[128];
    memset(chunkLine,0,sizeof(chunkLine));
    long chunkCount = 0;
    char *result = calloc(strlen(*io)+1,sizeof(char *));
    int i = 0;
    int j = 0;
    int k = 0;
    while(1){
        //je třeba načíst řádek s počtem načtených bytů
        if(chunkCount == 0 && (*io)[i] != '\n'){
            //narazili jsme na konec řádku, konvertujeme počet bytů z 16 soustavy na 10
            if((*io)[i] == '\r'){
                //printf("%s\n",chunkLine);
                chunkCount = strtol(chunkLine,NULL,16);
                memset(chunkLine,0,sizeof(chunkLine));
                i+=2;
                j = 0;
                continue;
            }
            //chunkování končí počtem chunků rovno 0
            else if((*io)[i] == '0' && (*io)[i-1] == '\n' && (*io)[i+1] == '\r') {
                break;
            }
            chunkLine[j] = (*io)[i];
            j++;
        }
        //čteme dokud počet načtených bytů není na 0
        while(chunkCount > 0) {
            result[k] = (*io)[i];
            chunkCount--;
            i++;
            k++;
        }
        i++;
    }
    free(*io);
    *io = result;
    return 0;
}

/**
 * Načte vrácený řetězec
 * @param bio - aktivní připojení
 * @param output - výstupní řetězec
 * @return 1 v případě chyby, jinak 0
 */
int processBioConn(BIO *bio,char **output){
    /* Send the request */
    char *tmpOut = calloc(2048,sizeof(char *));
    int skipFlag = 0;
    int chunkFlag = 0;
    int retVal;
    char tmpbuff[2048];
    memset(tmpbuff,0,sizeof(tmpbuff));

    while(1) {
        retVal = BIO_read(bio, tmpbuff, 2047);
        //pokud je vráceno 0 či méně, není co už číst
        if(retVal <= 0)
            break;
        //pokud ještě nebyla zkontrolována hlavička
        if(!skipFlag) {
            skipFlag = 1;
            int j = 0;
            //kontrola hlavičky
            int skip = analyseHeader(tmpbuff,&chunkFlag);
            //pokud je vráceno -1, není přečtena celá hlavička
            if(skip == -1) {
                skipFlag = 0;
                continue;
            }
            //pokud je vráceno -2, hlavička není správně
            else if(skip == -2){
                free(tmpOut);
                return 1;
            }
            //přeskočíme hlavičku a načteme zbytek znaků do bufferu
            for (int i = skip; i < strlen(tmpbuff); i++) {
                tmpOut[j] = tmpbuff[i];
                j++;
            }
            memset(tmpbuff,0,sizeof(tmpbuff));
            continue;
        }
        tmpOut = (char *)realloc(tmpOut,retVal+strlen(tmpOut)+1);
        strcat(tmpOut,tmpbuff);
        memset(tmpbuff,0,sizeof(tmpbuff));
    }
    //pokud bylo zapnuto chunkování, odstraníme chunky
    if(chunkFlag){
        uniteChunks(&tmpOut);
    }
    *output = tmpOut;
    return 0;
}

//utrzky prevzaty z ibm tutorialu TODO
/**
 * Připojení bez ssl
 * @param hostname - adresa hosta
 * @param fileAddr - část adresy (umístění souboru)
 * @param output - výstup (očekáváme xml)
 * @return 0 pokud je vše v pořádku, 31 při selhání
 */
int getNoSslFeed(char *hostname, char *fileAddr, char **output){
    BIO * bio;
    //požadavek na server
    char * request = calloc(128+strlen(hostname)+strlen(fileAddr),sizeof(char *));
    sprintf(request,"GET %s HTTP/1.1\r\nHost: %s\r\nConnection: Close\r\n\r\n",fileAddr, hostname);

    //Načtení knihoven
    ERR_load_BIO_strings();
    SSL_load_error_strings();
    OpenSSL_add_all_algorithms();

    //regex pro zjištění prezence portu
    regex_t port;
    regcomp(&port,"^.+:[0-9]+",REG_EXTENDED);
    if(regexec(&port,hostname,0,NULL,0))
        strcat(hostname,":80");
    //připravení BIO pro připojení
    bio = BIO_new_connect(hostname);
    regfree(&port);
    if(bio == NULL) {
        printf("BIO is null\n");
        ERR_free_strings();
        EVP_cleanup();
        free(request);
        return 32;
    }

    //připojení k serveru
    if(BIO_do_connect(bio) <= 0) {
        ERR_print_errors_fp(stderr);
        BIO_free_all(bio);
        ERR_free_strings();
        EVP_cleanup();
        free(request);
        return 32;
    }

    // zaslání požadavku
    if(BIO_write(bio, request, (int)strlen(request)) != strlen(request)){
        ERR_print_errors_fp(stderr);
        BIO_free_all(bio);
        ERR_free_strings();
        EVP_cleanup();
        free(request);
        return 32;
    }

    //přečtení výstupu
    if(processBioConn(bio,output)){
        ERR_print_errors_fp(stderr);
        BIO_free_all(bio);
        ERR_free_strings();
        EVP_cleanup();
        free(request);
        return 33;
    }

    //uvolnění
    BIO_free_all(bio);
    ERR_free_strings();
    EVP_cleanup();
    free(request);
    return 0;
}
//ibm tutorial TODO
/**
 * Připojení s ssl
 * @param hostname - adresa hosta
 * @param fileAddr - část adresy (umístění souboru)
 * @param certFile - cesta k souboru s certifikátem
 * @param certAddr - cesta k adresáři s certifikáty
 * @param output - výstup (očekáváme xml)
 * @return 0 pokud je vše v pořádku, 31 při selhání
 */
int getSslFeed(char *hostname, char *fileAddr, char *certFile, char *certAddr, char **output){
    BIO * bio;
    SSL * ssl;
    SSL_CTX * ctx;

    //požadavek na server
    char * request = calloc(128+strlen(hostname)+strlen(fileAddr),sizeof(char *));
    sprintf(request,"GET %s HTTP/1.1\r\nHost: %s\r\nUser-Agent: Mozilla/5.0\r\nConnection: Close\r\n\r\n", fileAddr, hostname);

    //Načtení knihoven
    SSL_library_init();
    ERR_load_BIO_strings();
    SSL_load_error_strings();
    OpenSSL_add_all_algorithms();

    //vytvoření SSL kontextu
    ctx = SSL_CTX_new(SSLv23_client_method());
    if(ctx == NULL){
        fprintf(stderr,"Chyba při inicializaci certifikátu\n");
        free(request);
        ERR_free_strings();
        EVP_cleanup();
        return 32;
    }
    //nastavení flagů (chceme SSLv23)
    const long flags = SSL_OP_NO_SSLv2 | SSL_OP_NO_SSLv3 | SSL_OP_NO_COMPRESSION;
    SSL_CTX_set_options(ctx, flags);

    //načtení certifikátů
    if(certFile == NULL && certAddr == NULL)
        SSL_CTX_set_default_verify_paths(ctx);
    else{
        SSL_CTX_load_verify_locations(ctx,certFile,certAddr);
    }

    //init BIO pro připojení
    bio = BIO_new_ssl_connect(ctx);

    BIO_get_ssl(bio, & ssl);
    SSL_set_mode(ssl, SSL_MODE_AUTO_RETRY);
    //regex pro výskyt portu v adrese
    regex_t port;
    regcomp(&port,"^.+:[0-9]+",REG_EXTENDED);
    if(regexec(&port,hostname,0,NULL,0))
        strcat(hostname,":https");
    //vytvoření připojení
    BIO_set_conn_hostname(bio, hostname);
    regfree(&port);
    //připojení
    if(BIO_do_connect(bio) <= 0)
    {
        fprintf(stderr, "Error attempting to connect\n");
        BIO_free_all(bio);
        SSL_CTX_free(ctx);
        ERR_free_strings();
        EVP_cleanup();
        return 32;
    }

    //Kontrola certifikátů
    if(SSL_get_verify_result(ssl) != X509_V_OK)
    {
        fprintf(stderr, "Certificate verification error: %ld\n", SSL_get_verify_result(ssl));
        BIO_free_all(bio);
        SSL_CTX_free(ctx);
        ERR_free_strings();
        EVP_cleanup();
        return 32;
    }

    //Zaslání požadavku
    if(BIO_write(bio, request, (int)strlen(request)) != strlen(request)){
        ERR_print_errors_fp(stderr);
        BIO_free_all(bio);
        ERR_free_strings();
        EVP_cleanup();
        free(request);
        return 32;
    }

    //přečtení výstupu
    if(processBioConn(bio,output)){
        ERR_print_errors_fp(stderr);
        BIO_free_all(bio);
        ERR_free_strings();
        EVP_cleanup();
        free(request);
        return 33;
    }

    //uvolnění
    free(request);
    BIO_free_all(bio);
    ERR_free_strings();
    EVP_cleanup();
    SSL_CTX_free(ctx);
    return 0;
}

/**
 * Vstupní bod
 * @param url - fronta adres
 * @param certFile - adresa k souboru s certifikátem
 * @param certAddr - adresa k adresáři s certifikáty
 * @param tFlag - flag čas
 * @param aFlag - flag autor
 * @param uFlag - flag url
 * @return 0 pokud je vše ok,
 *         31 při chybě v této funkci,
 *         32 při pokusu se připojit,
 *         33 při zpracování hlavičky a xml,
 *         5x při chybě v parseru
 */
int feedreader(TQueue *url, char *certFile, char *certAddr, int tFlag, int aFlag, int uFlag){

    while(url->front != NULL){
        int ssl = 0;
        char *activeUrl;

        //načtení url a zpracování pro požadavek na server
        QueueFrontPop(url,&activeUrl);
        size_t activeLen = strlen(activeUrl);
        char *token = strtok(activeUrl,"/");
        if(strcmp(token,"https:") == 0){
            ssl = 1;
        }
        else if (strcmp(token,"http:") != 0){
            fprintf(stderr,"Chyba spatna url");
            return 30;
        }

        token = strtok(NULL,"/");
        //uložení domény
        char *hostname = (char *)malloc(strlen(token)+10);
        strcpy(hostname,token);
        //uložení adresy uložení na serveru
        char *fileAddr = (char *)malloc(activeLen - strlen(hostname)+1);
        *fileAddr = 0;
        while((token = strtok(NULL,"/")) != NULL){
            strcat(fileAddr,"/");
            strcat(fileAddr,token);
        }

        char *output = 0;
        int retVal = 0;
        //připojení ssl nebo bez
        if(ssl){
            retVal = getSslFeed(hostname,fileAddr,certFile,certAddr,&output);
        } else{
            retVal = getNoSslFeed(hostname,fileAddr,&output);
        }

        if(retVal){
            return retVal;
        }

        //předáme výsledek požadavku parseru
        retVal = parsexml(output, tFlag, aFlag, uFlag);
        if(url->front != NULL)
            printf("\n");
        free(activeUrl);
        free(hostname);
        free(fileAddr);
        free(output);
        if(retVal && url->front == NULL)
            return retVal;
    }
    return 0;
}