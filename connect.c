//
// Created by jan on 12.10.18.
//

#include "connect.h"

//potřebuju pro druhy pruchod
int pass = 0;
int newLine = 0;
int ok = 0;

int analyseHeader(char *input,int *chunk){
    //printf("%s\n\n",input);
    //printf("%d %d\n",ok,pass);
    char *line = (char *)malloc(1024);
    memset(line,0,1024);
    regex_t contentType;
    regcomp(&contentType,"^Content-Type: .*/.*xml.*",REG_EXTENDED);
    int j = 0;
    for (int i = 0;; i++) {
        if((input[i] == '\r' && newLine)) {
            i+=2;
            free(line);
            regfree(&contentType);
            //printf("%d %d\n",ok,pass);
            if(pass)
                return i;
            return -2;
        }
        else if(input[i] == '\r'){
            //printf("%s\n",line);
            if(!ok){
                if(strcmp(line,"HTTP/1.1 200 OK") != 0){
                    free(line);
                    regfree(&contentType);
                    return -2;
                }
                ok = 1;
            }
            else if(strcmp(line,"Transfer-Encoding: chunked") == 0)
                *chunk = 1;
            else if(!regexec(&contentType,line,0,NULL,0))
                pass = 1;
            j = 0;
            //printf("%s\n",line);
            memset(line,0,128);
            continue;
        }
        else if(input[i] == '\n') {
            newLine = 1;
            continue;
        }
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

int uniteChunks(char **io){
    //printf("%s\n",*io);
    char chunkLine[128];
    memset(chunkLine,0,sizeof(chunkLine));
    long chunkCount = 0;
    char *result = (char *)malloc(strlen(*io)+1);
    memset(result,0,strlen(*io)+1);
    int i = 0;
    int j = 0;
    int k = 0;
    while(1){
        if(chunkCount == 0 && (*io)[i] != '\n'){
            if((*io)[i] == '\r'){
                //printf("%s\n",chunkLine);
                chunkCount = strtol(chunkLine,NULL,16);
                memset(chunkLine,0,sizeof(chunkLine));
                i+=2;
                j = 0;
                continue;
            }
            else if((*io)[i] == '0' && (*io)[i-1] == '\n' && (*io)[i+1] == '\r') {
                break;
            }
            chunkLine[j] = (*io)[i];
            j++;
        }
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

int processBioConn(BIO *bio,char **output){
    /* Send the request */
    char *tmpOut = (char *)malloc(2048);
    memset(tmpOut,0,2048);
    int skipFlag = 0;
    int chunkFlag = 0;
    int p;
    char tmpbuff[2048];
    memset(tmpbuff,0,sizeof(tmpbuff));


    /* Read in the response */
    for(;;) {
        p = BIO_read(bio, tmpbuff, 2047);
        //printf("%s\n",tmpbuff);
        if(p <= 0)
            break;
        if(!skipFlag) {
            skipFlag = 1;
            int j = 0;

            int skip = analyseHeader(tmpbuff,&chunkFlag);
            //printf("%d\n",skip);
            //printf("%d\n",p);
            //printf("%s\n",tmpbuff);
            if(skip == -1) {
                skipFlag = 0;
                continue;
            }
            else if(skip == -2){
                free(tmpOut);
                return 1;
            }
            for (int i = skip; i < strlen(tmpbuff); i++) {
                tmpOut[j] = tmpbuff[i];
                j++;
            }
            memset(tmpbuff,0,sizeof(tmpbuff));
            continue;
        }
        tmpOut = (char *)realloc(tmpOut,p+strlen(tmpOut)+1);
        strcat(tmpOut,tmpbuff);
        memset(tmpbuff,0,sizeof(tmpbuff));
    }
    /* Close the connection and free the context */
    //printf("%s\n",tmpOut);
    if(chunkFlag){
        uniteChunks(&tmpOut);
    }
    *output = tmpOut;
    return 0;
}

//utrzky prevzaty z ibm tutorialu TODO
int getNoSslFeed(char *hostname, char *fileAddr, char **output){
    BIO * bio;

    char * request = (char *)malloc(128+strlen(hostname)+strlen(fileAddr));
    sprintf(request,"GET %s HTTP/1.1\r\nHost: %s\r\nConnection: Close\r\n\r\n",fileAddr, hostname);
    /* Set up the library */

    ERR_load_BIO_strings();
    SSL_load_error_strings();
    OpenSSL_add_all_algorithms();

    /* Create and setup the connection */
    regex_t port;
    regcomp(&port,"^.+:[0-9]+",REG_EXTENDED);
    if(regexec(&port,hostname,0,NULL,0))
        strcat(hostname,":80");
    bio = BIO_new_connect(hostname);
    regfree(&port);
    if(bio == NULL) {
        printf("BIO is null\n");
        ERR_free_strings();
        EVP_cleanup();
        free(request);
        return -1;
    }

    if(BIO_do_connect(bio) <= 0) {
        ERR_print_errors_fp(stderr);
        BIO_free_all(bio);
        ERR_free_strings();
        EVP_cleanup();
        free(request);
        return -1;
    }

    if(BIO_write(bio, request, (int)strlen(request)) != strlen(request)){
        ERR_print_errors_fp(stderr);
        BIO_free_all(bio);
        ERR_free_strings();
        EVP_cleanup();
        free(request);
        return -1;
    }
    if(processBioConn(bio,output)){
        ERR_print_errors_fp(stderr);
        BIO_free_all(bio);
        ERR_free_strings();
        EVP_cleanup();
        free(request);
        return -1;
    }

    BIO_free_all(bio);
    ERR_free_strings();
    EVP_cleanup();
    free(request);
    return 0;
}

int getSslFeed(char *hostname, char *fileAddr, char *certFile, char *certAddr, char **output){
    BIO * bio;
    SSL * ssl;
    SSL_CTX * ctx;

    char * request = (char *)malloc(128+strlen(hostname)+strlen(fileAddr));
    sprintf(request,"GET %s HTTP/1.1\r\nHost: %s\r\nUser-Agent: Mozilla/5.0\r\nConnection: Close\r\n\r\n", fileAddr, hostname);
    /* Set up the library */

    //printf("%s\n",request);
    SSL_library_init();
    ERR_load_BIO_strings();
    SSL_load_error_strings();
    OpenSSL_add_all_algorithms();

    /* Set up the SSL context */
    ctx = SSL_CTX_new(SSLv23_client_method());
    if(ctx == NULL){
        fprintf(stderr,"Chyba při inicializaci certifikátu\n");
        free(request);
        ERR_free_strings();
        EVP_cleanup();
        return -1;
    }
    const long flags = SSL_OP_NO_SSLv2 | SSL_OP_NO_SSLv3 | SSL_OP_NO_COMPRESSION;
    SSL_CTX_set_options(ctx, flags);
    /* Load the trust store */
    if(certFile == NULL && certAddr == NULL)
        SSL_CTX_set_default_verify_paths(ctx);
    else{
        SSL_CTX_load_verify_locations(ctx,certFile,certAddr);
    }

    /* Setup the connection */

    bio = BIO_new_ssl_connect(ctx);

    /* Set the SSL_MODE_AUTO_RETRY flag */

    BIO_get_ssl(bio, & ssl);
    SSL_set_mode(ssl, SSL_MODE_AUTO_RETRY);
    /* Create and setup the connection */
    regex_t port;
    regcomp(&port,"^.+:[0-9]+",REG_EXTENDED);
    if(regexec(&port,hostname,0,NULL,0))
        strcat(hostname,":https");
    BIO_set_conn_hostname(bio, hostname);
    regfree(&port);
    if(BIO_do_connect(bio) <= 0)
    {
        fprintf(stderr, "Error attempting to connect\n");
        ERR_print_errors_fp(stderr);
        BIO_free_all(bio);
        SSL_CTX_free(ctx);
        return 0;
    }

    /* Check the certificate */

    if(SSL_get_verify_result(ssl) != X509_V_OK)
    {
        fprintf(stderr, "Certificate verification error: %ld\n", SSL_get_verify_result(ssl));
        BIO_free_all(bio);
        SSL_CTX_free(ctx);
        return 0;
    }

    /* Send the request */
    if(BIO_write(bio, request, (int)strlen(request)) != strlen(request)){
        ERR_print_errors_fp(stderr);
        BIO_free_all(bio);
        ERR_free_strings();
        EVP_cleanup();
        free(request);
        return -1;
    }

    /* Read in the response */
    if(processBioConn(bio,output)){
        ERR_print_errors_fp(stderr);
        BIO_free_all(bio);
        ERR_free_strings();
        EVP_cleanup();
        free(request);
        return -1;
    }

    /* Close the connection and free the context */
    free(request);
    BIO_free_all(bio);
    ERR_free_strings();
    EVP_cleanup();
    SSL_CTX_free(ctx);
    return 0;
}

int feedreader(TQueue *url, char *certFile, char *certAddr, int tFlag, int aFlag, int uFlag){

    while(url->front != NULL){
        int ssl = 0;
        char *activeUrl;

        QueueFrontPop(url,&activeUrl);
        size_t activeLen = strlen(activeUrl);
        char *token = strtok(activeUrl,"/");
        if(strcmp(token,"https:") == 0){
            ssl = 1;
        }
        else if (strcmp(token,"http:") != 0){
            //je mozne zadat bez http na zacatku?
            fprintf(stderr,"Chyba spatna url");
            return -1;
        }

        token = strtok(NULL,"/");
        //velikost adresy hosta + port num
        char *hostname = (char *)malloc(strlen(token)+10);
        strcpy(hostname,token);
        char *fileAddr = (char *)malloc(activeLen - strlen(hostname)+1);
        *fileAddr = 0;
        while((token = strtok(NULL,"/")) != NULL){
            strcat(fileAddr,"/");
            strcat(fileAddr,token);
        }
        char *output = 0;
        int retVal = 0;
        if(ssl){
            retVal = getSslFeed(hostname,fileAddr,certFile,certAddr,&output);
        } else{
            retVal = getNoSslFeed(hostname,fileAddr,&output);
        }

        if(retVal){
            return retVal;
        }

        //printf("%s\n",output);
        parsexml(output, tFlag, aFlag, uFlag);
        if(url->front != NULL)
            printf("\n");
        free(activeUrl);
        free(hostname);
        free(fileAddr);
        free(output);
    }
    return 0;
}