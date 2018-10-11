#include "feeder.h"

int analyseHeader(char *input,int *chunk){
    //printf("%s\n\n",input);
    char *line = (char *)malloc(128);
    memset(line,0,128);
    int newLine = 0;
    int j = 0;
    for (int i = 0;; i++) {
        if((input[i] == '\r' && newLine)) {
            i+=2;
            free(line);
            return i;
        }
        else if(input[i] == '\r'){
            //printf("%d\n",i);
            if(strcmp(line,"Transfer-Encoding: chunked") == 0)
                *chunk = 1;
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
            return i;
        }

        line[j] = input[i];
        j++;
        newLine = 0;
    }
}

int uniteChunks(char **io){
    char chunkLine[50];
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

void printItem(char *title, char *time, char *author, char *email,char *url, int tFlag, int aFlag, int uFlag){
    if(title == NULL){
        title = (char *)malloc(50);
        strcpy(title,"Chyba: chybí titulek u \"item\" ");
    }
    if(time == NULL){
        time = (char *)malloc(50);
        strcpy(time,"Chyba: chybí čas aktualizace u \"item\" ");
    }
    if(author == NULL && email == NULL){
        author = (char *)malloc(50);
        strcpy(author,"Chyba: chybí autor u \"item\" ");
    }
    if(url == NULL){
        url = (char *)malloc(50);
        strcpy(url,"Chyba: chybí url u \"item\" ");
    }
    printf("%s\n",title);

    for (int i = 1; i < 4; i++) {
        if(tFlag == i)
            printf("Aktualizace: %s\n",time);
        else if(uFlag == i)
            printf("URL: %s\n",url);
        else if(aFlag == i) {
            if(email == NULL)
                printf("Autor: %s\n", author);
            else if(author == NULL)
                printf("Email: %s\n", email);
            else
                printf("Autor: %s\nEmail: %s\n", author,email);
        }
    }
    if(tFlag || aFlag || uFlag)
        printf("\n");

    free(title);
    free(time);
    free(author);
    free(url);
    if(email != NULL)
        free(email);
}

int rssLegacyProcessItem(xmlNode *rssItem, int tFlag, int aFlag, int uFlag){
    char *title = NULL;
    char *time = NULL;
    char *author = NULL;
    char *url = NULL;

    for (xmlNode *actItem = rssItem; actItem != NULL ; actItem = actItem->next) {
        //printf("%s\n",actItem->name);
        if(xmlStrcmp(actItem->name,(xmlChar *)"title") == 0){
            title = (char *)malloc(strlen((char *)actItem->children->content)+1);
            strcpy(title,(char *)actItem->children->content);
        }
        else if(xmlStrcmp(actItem->name,(xmlChar *)"") == 0){
            time = (char *)malloc(strlen((char *)actItem->children->content)+1);
            strcpy(time,(char *)actItem->children->content);
        }
        else if(xmlStrcmp(actItem->name,(xmlChar *)"link") == 0){
            url = (char *)malloc(strlen((char *)actItem->children->content)+1);
            strcpy(url,(char *)actItem->children->content);
        }
        else if(xmlStrcmp(actItem->name,(xmlChar *)"") == 0){
            //TODO najít pridani autora
        }
    }

    printItem(title,time,author,NULL,url,tFlag,aFlag,uFlag);
    return 0;
}

int parseLegacyRss(xmlNode *node, int tFlag, int aFlag, int uFlag){
    if(node == NULL){
        fprintf(stderr,"Chyba, nespravny format xml\n");
        return 1;
    }
    for(xmlNode *actNode = node; actNode != NULL;actNode = actNode->next){
        if(xmlStrcmp(actNode->name,(xmlChar *)"channel") == 0) {
            for (xmlNode *actChannel = actNode->children; actChannel != NULL ; actChannel = actChannel->next) {
                if(xmlStrcmp(actNode->name,(xmlChar *)"title") == 0){
                    //if pro chybu
                    printf("*** %s ***\n",actNode->children->content);
                    break;
                }
            }
        }
        else if(xmlStrcmp(actNode->name,(xmlChar *)"item") == 0){
            rssLegacyProcessItem(actNode->children,tFlag,aFlag,uFlag);
        }

    }

    return 0;
}

int rssProcessItem(xmlNode *rssItem, int tFlag, int aFlag, int uFlag){
    char *title = NULL;
    char *time = NULL;
    char *author = NULL;
    char *url = NULL;

    for (xmlNode *actItem = rssItem; actItem != NULL ; actItem = actItem->next) {
        //printf("%s\n",actItem->name);
        if(xmlStrcmp(actItem->name,(xmlChar *)"title") == 0){
            title = (char *)malloc(strlen((char *)actItem->children->content)+1);
            strcpy(title,(char *)actItem->children->content);
        }
        else if(xmlStrcmp(actItem->name,(xmlChar *)"pubDate") == 0){
            time = (char *)malloc(strlen((char *)actItem->children->content)+1);
            strcpy(time,(char *)actItem->children->content);
        }
        else if(xmlStrcmp(actItem->name,(xmlChar *)"guid") == 0){
            url = (char *)malloc(strlen((char *)actItem->children->content)+1);
            strcpy(url,(char *)actItem->children->content);
        }
        else if(xmlStrcmp(actItem->name,(xmlChar *)"author") == 0){
            author = (char *)malloc(strlen((char *)actItem->children->content)+1);
            strcpy(author,(char *)actItem->children->content);
        }
    }

    printItem(title,time,author,NULL,url,tFlag,aFlag,uFlag);
    return 0;
}

int parseRss(xmlNode *node, int tFlag, int aFlag, int uFlag){
    while(xmlStrcmp(node->name,(xmlChar *)"channel") != 0 && node != NULL){
        node = node->next;
    }
    if(node == NULL){
        fprintf(stderr,"Chyba, nespravny format xml\n");
        return 1;
    }
    node = node->children;
    for(xmlNode *actNode = node; actNode != NULL;actNode = actNode->next){
        if(xmlStrcmp(actNode->name,(xmlChar *)"title") == 0){
            //if pro chybu
            printf("*** %s ***\n",actNode->children->content);
        }
        else if(xmlStrcmp(actNode->name,(xmlChar *)"item") == 0){
            rssProcessItem(actNode->children,tFlag,aFlag,uFlag);
        }

    }

    return 0;
}

int atomProcessEntry(xmlNode *rssItem, int tFlag, int aFlag, int uFlag){
    char *title = NULL;
    char *time = NULL;
    char *author = NULL;
    char *email = NULL;
    char *url = NULL;

    for (xmlNode *actItem = rssItem; actItem != NULL ; actItem = actItem->next) {
        //printf("%s\n",actItem->name);
        if(xmlStrcmp(actItem->name,(xmlChar *)"title") == 0){
            title = (char *)malloc(strlen((char *)actItem->children->content)+1);
            strcpy(title,(char *)actItem->children->content);
        }
        else if(xmlStrcmp(actItem->name,(xmlChar *)"updated") == 0){
            time = (char *)malloc(strlen((char *)actItem->children->content)+1);
            strcpy(time,(char *)actItem->children->content);
        }
        else if(xmlStrcmp(actItem->name,(xmlChar *)"link") == 0){
            url = (char *)malloc(strlen((char *)actItem->properties->children->content)+1);
            strcpy(url,(char *)actItem->properties->children->content);
        }
        else if(xmlStrcmp(actItem->name,(xmlChar *)"author") == 0){
            for (xmlNode *actAuthor = actItem->children; actAuthor != NULL; actAuthor = actAuthor->next) {
                if(xmlStrcmp(actItem->name,(xmlChar *)"name") == 0){
                    author = (char *)malloc(strlen((char *)actItem->properties->children->content)+1);
                    strcpy(author,(char *)actItem->properties->children->content);
                }
                else if(xmlStrcmp(actItem->name,(xmlChar *)"email") == 0){
                    email = (char *)malloc(strlen((char *)actItem->properties->children->content)+1);
                    strcpy(email,(char *)actItem->properties->children->content);
                }
            }
        }
    }

    printItem(title,time,author,email,url,tFlag,aFlag,uFlag);
    return 0;
}

int parseAtom(xmlNode *node, int tFlag, int aFlag, int uFlag){
    if(node == NULL){
        fprintf(stderr,"Chyba, nespravny format xml\n");
        return 1;
    }
    for(xmlNode *actNode = node; actNode != NULL;actNode = actNode->next){
        //printf("%s\n",actNode->name);
        if(xmlStrcmp(actNode->name,(xmlChar *)"title") == 0){
            //if pro chybu
            printf("*** %s ***\n",actNode->children->content);
        }
        else if(xmlStrcmp(actNode->name,(xmlChar *)"entry") == 0){
            atomProcessEntry(actNode->children,tFlag,aFlag,uFlag);
        }

    }

    return 0;
}

//prevzato z xml examles tree 1 TODO
int parsexml(char *input, int tFlag, int aFlag, int uFlag){
    xmlDoc *feed = NULL;
    xmlNode *root = NULL;
    printf("\n");

    feed = xmlReadDoc((xmlChar *)input,"newurl.org",NULL,0);

    root = xmlDocGetRootElement(feed);
    if(xmlStrcmp(root->name,(xmlChar *)"rss") == 0){
        parseRss(root->children, tFlag,aFlag,uFlag);
    }
    else if(xmlStrcmp(root->name,(xmlChar *)"feed") == 0){
        parseAtom(root->children, tFlag,aFlag,uFlag);
    }
    else if(xmlStrcmp(root->name,(xmlChar *)"rdf:RDF") == 0){
        parseLegacyRss(root->children, tFlag,aFlag,uFlag);
    }
    printf("\n");
    xmlFreeDoc(feed);
    xmlCleanupParser();
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
            if(skip == p && p == 2047) {
                skipFlag = 0;
                continue;
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
    strcat(hostname,":80");
    bio = BIO_new_connect(hostname);
    if(bio == NULL) {
        printf("BIO is null\n");
        return -1;
    }

    if(BIO_do_connect(bio) <= 0) {
        ERR_print_errors_fp(stderr);
        BIO_free_all(bio);
        return -1;
    }
    BIO_write(bio, request, strlen(request));
    processBioConn(bio,output);

    BIO_free_all(bio);
    ERR_remove_state(0);
    free(request);
    return 0;
}

int getSslFeed(char *hostname, char *fileAddr, TQueue *cert, int certFlag, char **output){
    BIO * bio;
    SSL * ssl;
    SSL_CTX * ctx;

    char * request = (char *)malloc(128+strlen(hostname)+strlen(fileAddr));
    sprintf(request,"GET %s HTTP/1.1\r\nHost: %s\r\nUser-Agent: Mozilla/5.0\r\nConnection: Close\r\n\r\n", fileAddr, hostname);
    /* Set up the library */

    printf("%s\n",request);
    SSL_library_init();
    ERR_load_BIO_strings();
    SSL_load_error_strings();
    //OpenSSL_add_all_algorithms();

    /* Set up the SSL context */
    ctx = SSL_CTX_new(SSLv23_client_method());
    /* Load the trust store */
    if(!certFlag)
        SSL_CTX_set_default_verify_paths(ctx);
    else{
        while(cert->front != NULL){
            char *queueStr;
            QueueFrontPop(cert,&queueStr);
            SSL_CTX_load_verify_locations(ctx,queueStr,0);
            free(queueStr);
        }
    }

    /* Setup the connection */

    bio = BIO_new_ssl_connect(ctx);

    /* Set the SSL_MODE_AUTO_RETRY flag */

    BIO_get_ssl(bio, & ssl);
    SSL_set_mode(ssl, SSL_MODE_AUTO_RETRY);

    /* Create and setup the connection */
    strcat(hostname,":https");
    BIO_set_conn_hostname(bio, hostname);
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
    BIO_write(bio, request, strlen(request));

    /* Read in the response */
    processBioConn(bio,output);

    /* Close the connection and free the context */
    free(request);
    BIO_free_all(bio);
    ERR_remove_state(0);
    SSL_CTX_free(ctx);
    return 0;
}

int feedreader(TQueue *url, TQueue *cert, int certFlag, int tFlag, int aFlag, int uFlag){

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

        if(ssl){
            getSslFeed(hostname,fileAddr,cert,certFlag,&output);
        } else{
            getNoSslFeed(hostname,fileAddr,&output);
        }

        //printf("%s\n",output);

        parsexml(output, tFlag, aFlag, uFlag);
        free(activeUrl);
        free(hostname);
        free(fileAddr);
        free(output);
    }
    return 0;
}