/**
 * Kód - Parser pro feedy Atom a RSS
 * Autor: Jan Vávra
 * Login: xvavra20
 */

#include "feeder.h"

// flagy pro jednotlivé typy feedů
int rss, rdf, atom;
//fronta pro výstupní text
TQueue *text;


/**
 * Funkce zbaví element přípony, aby reader mohl fungovat i bez namespace argumentu
 * @param node - uzel, který může obsahovat příponu
 */
void removeColom(xmlNode *node){
    regex_t colom;
    regcomp(&colom,".*:.*",REG_EXTENDED);
    if(!regexec(&colom,(char *)node->name,0,NULL,0)){
        char *tmp = calloc(strlen((char *)node->name)+1,sizeof(char *));
        strcpy(tmp,(char *)node->name);
        char *token = strtok(tmp,":");
        token = strtok(NULL,":");
        strcpy((char *)node->name,token);
        free(tmp);
    }
    regfree(&colom);
}

/**
 * Zkontroluje kořenový uzel
 * @param root - kořenový uzel
 * @return 1 v případě chyby, jinak 0
 */
int checkRoot(xmlNode *root){
    removeColom(root);
    //kontrola typu feedu
    if(xmlStrcmp(root->name,(xmlChar *)"rss") == 0)
        rss = 1;
    else if(xmlStrcmp(root->name,(xmlChar *)"feed") == 0)
        atom = 1;
    else if(xmlStrcmp(root->name,(xmlChar *)"RDF") == 0)
        rdf = 1;
    else
        return 1;
    return 0;

}

/**
 * Zkontrolování a načtení text uzlu rodičovského uzlu
 * @param node - rodičovský uzel, který je třeba zkontrolovat
 * @param str - výstupní řetězec funkce
 * @return 1 v případě chyby, jinak 0
 */
int loadValue(xmlNode *node, char **str){
    //kontrola, že uzel něco obsahuje
    if(node->children != NULL && node->children->next == NULL && node->children->content != NULL) {
        //pokud je str NULL, oznámíme že uzel je bezpečný
        if (str == NULL) {
            return 0;
        }
        if (*str == NULL){
            //alokace a vrácení uzlu
            *str = (char *) malloc(strlen((char *) node->children->content) + 21);
            strcpy(*str, (char *) node->children->content);
        }
        return 0;
    }
    else{
        return 1;
    }
}

/**
 * Načte výstupní řetězce článku (item/entry) do fronty
 * @param title - Nadpis článku
 * @param time - Čas úpravy
 * @param author - Autor článku
 * @param email - Email autora (Atom)
 * @param url - URL ke článku
 * @param tFLag - flag, jestli máme zahrnout čas do výstupu
 * @param aFLag - flag, jestli máme zahrnout autora do výstupu
 * @param uFLag - flag, jestli máme zahrnout URL do výstupu
 */
void printItem(char *title, char *time, char *author, char *email,char *url, int tFlag, int aFlag, int uFlag){
    //pokud item nemá žádný element, nemá cenu ho vypisovat
    if(title == NULL && time == NULL && author == NULL && url == NULL){
        return;
    }
    //pokud chybí, načteme oznamující řetězec o nedostupnosti elementu
    if(title == NULL){
        title = (char *)malloc(50);
        strcpy(title,"Bez názvu");
    }
    else if(time == NULL){
        time = (char *)malloc(50);
        strcpy(time,"chybí čas aktualizace");
    }
    if(author == NULL && email == NULL){
        author = (char *)malloc(50);
        strcpy(author,"chybí autor");
    }
    if(url == NULL){
        url = (char *)malloc(50);
        strcpy(url,"chybí url");
    }
    //načteme Nadpis (povinný pro všechny feedy)
    QueueUp(text,title);

    //poté načteme doplňující informace v pořadí flagů na vstupu
    for (int i = 1; i < 4; i++) {
        char *tmp;
        if(tFlag == i) {
            tmp = calloc(strlen(time)+1,sizeof(char *));
            sprintf(tmp,"Aktualizace: %s",time);
            QueueUp(text, tmp);
            free(tmp);
        }
        else if(uFlag == i){
            tmp = calloc(strlen(url)+1,sizeof(char *));
            sprintf(tmp,"URL: %s",url);
            QueueUp(text, tmp);
            free(tmp);
        }
        else if(aFlag == i) {
            if(author != NULL && email == NULL){
                tmp = calloc(strlen(author)+1,sizeof(char *));
                sprintf(tmp,"Autor: %s",author);
                QueueUp(text, tmp);
                free(tmp);
            }
            else if(email != NULL && author == NULL){
                tmp = calloc(strlen(email)+1,sizeof(char *));
                sprintf(tmp,"Autor: %s",email);
                QueueUp(text, tmp);
                free(tmp);
            }
            else{
                tmp = calloc(strlen(author)+strlen(email)+1,sizeof(char *));
                sprintf(tmp,"Autor: %s %s",author,email);
                QueueUp(text, tmp);
                free(tmp);
            }
        }
    }
    //zařádkujeme
    if(tFlag || aFlag || uFlag)
        QueueUp(text,"");
    //uvolníme
    free(title);
    free(time);
    free(author);
    free(url);
    if(email != NULL)
        free(email);
}

 /**
  * Zpracuje item/entry feedu
  * @param rssItem - uzel itemu/etry
  * @param tFLag - flag, jestli máme zahrnout čas do výstupu
  * @param aFLag - flag, jestli máme zahrnout autora do výstupu
  * @param uFLag - flag, jestli máme zahrnout URL do výstupu
  * @return 1 v případě, že item/entry nemá požadovené elementy, jiank 0 (OK)
  */
int processItem(xmlNode *rssItem, int tFlag, int aFlag, int uFlag){
    char *title = NULL;
    char *time = NULL;
    char *author = NULL;
    char *email = NULL;
    char *url = NULL;
    if(rdf == 1) {
        for (xmlAttr *parrentAttr = rssItem->parent->properties; parrentAttr != NULL; parrentAttr = parrentAttr->next) {
            removeColom((xmlNode *)parrentAttr);
            if (xmlStrcmp(parrentAttr->name, (xmlChar *) "about") == 0) {
                if (loadValue((xmlNode *) parrentAttr, &url))
                    return 1;
            }

        }
    }
    for (xmlNode *actItem = rssItem; actItem != NULL ; actItem = actItem->next) {
        removeColom(actItem);
        //nadpis
        if(xmlStrcmp(actItem->name,(xmlChar *)"title") == 0){
            if(loadValue(actItem,&title))
                continue;
        }
        //autor DC
        else if(xmlStrcmp(actItem->name,(xmlChar *)"creator") == 0){
            if(loadValue(actItem,&author))
                continue;
        }
        else if(xmlStrcmp(actItem->name,(xmlChar *)"modified") == 0){
            if(loadValue(actItem,&time))
                continue;
        }
        // čas aktualizace RSS 2.0
        else if(xmlStrcmp(actItem->name,(xmlChar *)"pubDate") == 0){
            if(loadValue(actItem,&time))
                continue;
        }
        //možné URL RSS 2.0
        else if(xmlStrcmp(actItem->name,(xmlChar *)"guid") == 0){
            int isPermaLink = 0;
            for (xmlAttr *actGUID = actItem->properties; actGUID != NULL ; actGUID = actGUID->next) {
                removeColom((xmlNode *)actGUID);
                if(xmlStrcmp(actGUID->name,(xmlChar *)"isPermaLink") == 0) {
                    if (loadValue((xmlNode *) actGUID, NULL))
                        break;
                    if (xmlStrcmp(actGUID->children->content, (xmlChar *) "true") == 0)
                        isPermaLink = 1;
                }
            }
            if(isPermaLink){
                if(url != NULL) {
                    free(url);
                    url = NULL;
                }
                if(loadValue(actItem,&url))
                    continue;
            }
        }
        // čas aktualizace Atom
        else if(xmlStrcmp(actItem->name,(xmlChar *)"updated") == 0){
            if(loadValue(actItem,&time))
                continue;
        }
        // url RSS 1.0,2.0, Atom
        else if(xmlStrcmp(actItem->name,(xmlChar *)"link") == 0){
            if(actItem->properties == NULL){
                if(loadValue(actItem,&url))
                    continue;
            }
            for (xmlAttr *actUrl = actItem->properties; atom && actUrl != NULL ; actUrl = actUrl->next) {
                if(xmlStrcmp(actUrl->name,(xmlChar *)"href") == 0){
                    if(loadValue((xmlNode *)actUrl,&url))
                        continue;
                }
            }
        }
        //autor Atom, RSS 2.0
        else if(xmlStrcmp(actItem->name,(xmlChar *)"author") == 0){
            for (xmlNode *actAuthor = actItem->children; actAuthor != NULL; actAuthor = actAuthor->next) {
                removeColom(actAuthor);
                if(xmlStrcmp(actAuthor->name,(xmlChar *)"name") == 0){
                    if(author == NULL && loadValue(actAuthor,&author))
                        continue;
                }
                else if(xmlStrcmp(actAuthor->name,(xmlChar *)"email") == 0){
                    if(email == NULL && loadValue(actAuthor,&email))
                        continue;
                }
                else if(xmlStrcmp(actAuthor->name,(xmlChar *)"text") == 0){
                    if(author == NULL && loadValue(actItem,&author))
                        continue;
                }
            }
        }
    }

    printItem(title, time, author, email, url, tFlag, aFlag, uFlag);
    return 0;
 }

/**
 * Zpracuje typ RSS 1.0
 * @param node - uzel xml stromu
 * @param tFLag - flag, jestli máme zahrnout čas do výstupu
 * @param aFLag - flag, jestli máme zahrnout autora do výstupu
 * @param uFLag - flag, jestli máme zahrnout URL do výstupu
 * @return 0 pokud je vše v pořádku,
 *         52 při chybě před elementem item
 *         53 při chybě v elementu item
 */
int parseRdf(xmlNode *node, int tFlag, int aFlag, int uFlag){
    if(node == NULL){
        fprintf(stderr,"Chyba, nespravny format xml\n");
        return 52;
    }

    int title = 0;
    int noItem = 1;

    for(xmlNode *actNode = node; actNode != NULL;actNode = actNode->next){
        removeColom(actNode);
        if(xmlStrcmp(actNode->name,(xmlChar *)"channel") == 0) {
            for (xmlNode *actChannel = actNode->children; actChannel != NULL ; actChannel = actChannel->next) {
                removeColom(actChannel);
                //nadpis celého feedu
                if(xmlStrcmp(actChannel->name,(xmlChar *)"title") == 0){
                    title = 1;
                    if(loadValue(actChannel,NULL))
                        printf("*** Bez názvu ***\n");
                    else
                        printf("*** %s ***\n",(char *)actChannel->children->content);
                }
            }
        }
        //položka (item)
        else if(xmlStrcmp(actNode->name,(xmlChar *)"item") == 0){
            if(processItem(actNode->children,tFlag,aFlag,uFlag)) {
                break;
            }
            noItem = 0;
        }

    }

    if(!title && noItem) {
        fprintf(stderr,"Chyba, xml neobsahuje žádné data\n");
        return 52;
    }
    else if(!title)
        printf("*** Bez názvu ***\n");


    while(text->front != NULL){
        char *print;
        QueueFrontPop(text,&print);
        printf("%s\n",print);
        free(print);
    }
    return 0;
}

/**
 * Zpracuje typ RSS 2.0
 * @param node - uzel xml stromu
 * @param tFLag - flag, jestli máme zahrnout čas do výstupu
 * @param aFLag - flag, jestli máme zahrnout autora do výstupu
 * @param uFLag - flag, jestli máme zahrnout URL do výstupu
 * @return 0 pokud je vše v pořádku,
 *         52 při chybě před elementem item
 *         53 při chybě v elementu item
 */
int parseRss(xmlNode *node, int tFlag, int aFlag, int uFlag){
    removeColom(node);
    while(node != NULL && xmlStrcmp(node->name,(xmlChar *)"channel") != 0){
        node = node->next;
        removeColom(node);
    }
    if(node == NULL){
        fprintf(stderr,"Chyba, nespravny format xml\n");
        return 52;
    }

    int title = 0;
    int noItem = 1;

    node = node->children;
    for(xmlNode *actNode = node; actNode != NULL;actNode = actNode->next){
        removeColom(actNode);
        //nadpis celého feedu
        if(xmlStrcmp(actNode->name,(xmlChar *)"title") == 0 ){
            title = 1;
            if(loadValue(actNode,NULL))
                printf("*** Bez názvu ***\n");
            else
                printf("*** %s ***\n",(char *)actNode->children->content);

        }
        //položka (item)
        else if(xmlStrcmp(actNode->name,(xmlChar *)"item") == 0){
            if(processItem(actNode->children,tFlag,aFlag,uFlag)) {
                break;
            }
            noItem = 0;
        }
    }

    if(!title && noItem) {
        fprintf(stderr,"Chyba, xml neobsahuje žádné data\n");
        return 52;
    }
    else if(!title)
        printf("*** Bez názvu ***\n");


    while(text->front != NULL){
        char *print;
        QueueFrontPop(text,&print);
        printf("%s\n",print);
        free(print);
    }
    return 0;
}

/**
 * Zpracuje typ Atom
 * @param node - uzel xml stromu
 * @param tFLag - flag, jestli máme zahrnout čas do výstupu
 * @param aFLag - flag, jestli máme zahrnout autora do výstupu
 * @param uFLag - flag, jestli máme zahrnout URL do výstupu
 * @return 0 pokud je vše v pořádku,
 *         52 při chybě před elementem entry
 *         53 při chybě v elementu entry
 */
int parseAtom(xmlNode *node, int tFlag, int aFlag, int uFlag){
    if(node == NULL){
        fprintf(stderr,"Chyba, nespravny format xml\n");
        return 52;
    }
    int title = 0;
    int noItem = 1;

    for(xmlNode *actNode = node; actNode != NULL;actNode = actNode->next){
        removeColom(actNode);
        //nadpis celého feedu
        if(xmlStrcmp(actNode->name,(xmlChar *)"title") == 0){
            title = 1;
            if(loadValue(actNode,NULL))
                printf("*** Bez názvu ***\n");
            else
                printf("*** %s ***\n",(char *)actNode->children->content);
        }
        //položka (entry)
        else if(xmlStrcmp(actNode->name,(xmlChar *)"entry") == 0){
            if(processItem(actNode->children,tFlag,aFlag,uFlag)){
                break;
            }
            noItem = 0;
        }

    }

    if(!title && noItem) {
        fprintf(stderr,"Chyba, xml neobsahuje žádné data\n");
        return 52;
    }
    else if(!title)
        printf("*** Bez názvu ***\n");


    while(text->front != NULL){
        char *print;
        QueueFrontPop(text,&print);
        printf("%s\n",print);
        free(print);
    }
    return 0;
}

/**
 * Vstupní bod do parseru xml
 * @param input - vstupní xml
 * @param tFLag - flag, jestli máme zahrnout čas do výstupu
 * @param aFLag - flag, jestli máme zahrnout autora do výstupu
 * @param uFLag - flag, jestli máme zahrnout URL do výstupu
 * @return 0 pokud je vše v pořádku,
 *         51 při chybě v načítání nebo v kořeni xml
 *         52 při chybě v těle před item/entry
 *         53 při chybě uvnitř item/entry
 */
int parsexml(char *input, int tFlag, int aFlag, int uFlag){
    rss = 0;
    atom = 0;
    rdf = 0;
    int retVal = 0;
    text = malloc(sizeof(TQueue));
    QueueInit(text);
    xmlDoc *feed = NULL;
    xmlNode *root = NULL;

    feed = xmlReadDoc((xmlChar *)input,"newurl.org",NULL,XML_PARSE_NOWARNING|XML_PARSE_NOERROR|XML_PARSE_RECOVER);
    root = xmlDocGetRootElement(feed);


    //pokud je kořen NULL, nepodařilo se načíst řetězec do stromu
    if(root == NULL) {
        fprintf(stderr, "Chyba při načítání xml, špatný formát\n");
        QueueDestroy(text);
        free(text);
        xmlFreeDoc(feed);
        xmlCleanupParser();

        return 51;
    }
    if(checkRoot(root)){
        printf("Chyba: kořen neobsahuje požadované atributy\n");
        QueueDestroy(text);
        free(text);
        xmlFreeDoc(feed);
        xmlCleanupParser();
        return 51;
    }
    //pokud je flag 1, je primárně ten druh feedu
    if(rss == 1){
        //printf("%d\n",feed->properties);
        retVal = parseRss(root->children, tFlag,aFlag,uFlag);
    }
    else if(atom == 1){
        //printf("%d\n",feed->properties);
        retVal = parseAtom(root->children, tFlag,aFlag,uFlag);
    }
    else if(rdf == 1){
        //printf("%d\n",feed->properties);
        retVal = parseRdf(root->children, tFlag,aFlag,uFlag);
    }
    else{
        fprintf(stderr,"Chyba při rozpoznávání xml templatu\n");
        return 51;
    }
    QueueDestroy(text);
    free(text);
    xmlFreeDoc(feed);
    xmlCleanupParser();
    return retVal;
}
