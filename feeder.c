/**
 * Kód - Parser pro feedy Atom a RSS
 * Autor: Jan Vávra
 * Login: xvavra20
 */

#include "feeder.h"

// flagy pro jednotlivé typy feedů
int rss, rdf, atom, dc;
//fronta pro výstupní text
TQueue *text;

/**
 * Zkontroluje kořenový uzel
 * @param root - kořenový uzel
 * @return 1 v případě chyby, jinak 0
 */
int checkRoot(xmlNode *root){
    int rssOn = 0;
    int rdfOn = 0;
    int atomOn = 0;
    //kontrola typu feedu
    if(xmlStrcmp(root->name,(xmlChar *)"rss") == 0)
        rssOn = 1;
    else if(xmlStrcmp(root->name,(xmlChar *)"feed") == 0)
        atomOn = 1;
    else if(xmlStrcmp(root->name,(xmlChar *)"RDF") == 0)
        rdfOn = 1;
    else
        return 1;
    //kontrola na povinný atribut version
    if(rssOn){
        for (xmlAttr *actNode = root->properties; actNode != NULL ; actNode = actNode->next) {
            if(xmlStrcmp(actNode->name,(xmlChar *)"version") == 0){
                rss = 1;
            }
        }
    }
    //kontrola namespace, atom a rss 1.0 mají povinný, jinak zaznačíme, že můžeme používat elementy
    for (xmlNs *namespace = root->nsDef; namespace != NULL; namespace = namespace->next) {
        //Atom
        if(xmlStrcmp(namespace->href,(xmlChar *)"http://www.w3.org/2005/Atom") == 0){
            if(atom)
                return 1;
            else if(atomOn)
                atom = 1;
            else
                atom = 2;
        }
        //RSS 1.0
        else if(xmlStrcmp(namespace->href,(xmlChar *)"http://purl.org/rss/1.0/") == 0){
            if(rdf)
                return 1;
            else if(rdfOn)
                rdf = 1;
            else
                rdf = 2;
        }
        //RSS 2.0 (nepovinný)
        else if(xmlStrcmp(namespace->href,(xmlChar *)"http://www.rssboard.org/rss-specification") == 0){
            if(rssOn)
                return 1;
            else
                rss = 2;
        }
        //Dublin Core (nepovinný)
        else if(xmlStrcmp(namespace->href,(xmlChar *)"http://purl.org/dc/elements/1.1/") == 0){
            dc = 2;
        }
    }
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
    if(node->children != NULL && (xmlStrcmp(node->children->name,(xmlChar *)"text") == 0)
       && node->children->next == NULL && node->children->content != NULL) {
        //pokud je str NULL, oznámíme že uzel je bezpečný
        if (str == NULL) {
            return 0;
        }
        //alokace a vrácení uzlu
        *str = (char *) malloc(strlen((char *) node->children->content) + 21);
        strcpy(*str, (char *) node->children->content);
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
    //pokud chybí, načteme oznamující řetězec o nedostupnosti elementu
    if(time == NULL){
        time = (char *)malloc(50);
        strcpy(time,"chybí čas aktualizace u \"item\" ");
    }
    if(author == NULL && email == NULL){
        author = (char *)malloc(50);
        strcpy(author,"chybí autor u \"item\" ");
    }
    if(url == NULL){
        url = (char *)malloc(50);
        strcpy(url,"chybí url u \"item\" ");
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
            if(author != NULL){
                tmp = calloc(strlen(author)+1,sizeof(char *));
                sprintf(tmp,"Autor: %s",author);
                QueueUp(text, tmp);
                free(tmp);
            }
            if(email != NULL){
                tmp = calloc(strlen(email)+1,sizeof(char *));
                sprintf(tmp,"Email: %s",email);
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
    int atomEntryReq = 0;
    int rssItemReq = 0;
    int rdfItemReq = 0;
    for (xmlNode *actItem = rssItem; actItem != NULL ; actItem = actItem->next) {
        //nadpis
        if(xmlStrcmp(actItem->name,(xmlChar *)"title") == 0){
            if(loadValue(actItem,&title))
                continue;
            atomEntryReq++;
            rssItemReq++;
            rdfItemReq++;
        }
        //autor DC
        else if(xmlStrcmp(actItem->name,(xmlChar *)"creator") == 0 && dc){
            if(loadValue(actItem,&author))
                continue;
        }
        // čas aktualizace RSS 2.0
        else if(xmlStrcmp(actItem->name,(xmlChar *)"pubDate") == 0 && rss){
            if(loadValue(actItem,&time))
                continue;
        }
        //možné URL RSS 2.0
        else if(xmlStrcmp(actItem->name,(xmlChar *)"guid") == 0 && rss){
            int isPermaLink = 0;
            for (xmlAttr *actGUID = actItem->properties; actGUID != NULL ; actGUID = actGUID->next) {
                if(xmlStrcmp(actGUID->name,(xmlChar *)"isPermaLink") == 0) {
                    if (loadValue((xmlNode *) actGUID, NULL))
                        break;
                    if (xmlStrcmp(actGUID->children->content, (xmlChar *) "true") == 0)
                        isPermaLink = 1;
                }
            }
            if(isPermaLink){
                if(url != NULL)
                    free(url);
                if(loadValue(actItem,&url))
                    continue;
            }
        }
        // čas aktualizace Atom
        else if(xmlStrcmp(actItem->name,(xmlChar *)"updated") == 0 && atom){
            if(loadValue(actItem,&time))
                continue;
            atomEntryReq++;
        }
        // url RSS 1.0,2.0, Atom
        else if(xmlStrcmp(actItem->name,(xmlChar *)"link") == 0){
            if((rss || rdf ) && actItem->properties == NULL){
                if(loadValue(actItem,&url))
                    continue;
                rdfItemReq++;
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
                if(xmlStrcmp(actAuthor->name,(xmlChar *)"name") == 0 && atom){
                    if(author == NULL && loadValue(actAuthor,&author))
                        continue;
                }
                else if(xmlStrcmp(actAuthor->name,(xmlChar *)"email") == 0 && atom){
                    if(email == NULL && loadValue(actAuthor,&email))
                        continue;
                }
                else if(xmlStrcmp(actAuthor->name,(xmlChar *)"text") == 0 && rss){
                    if(author == NULL && loadValue(actAuthor,&author))
                        continue;
                }
            }
        }
        //id Atom (povinne)
        else if(xmlStrcmp(actItem->name,(xmlChar *)"id") == 0){
            if(loadValue(actItem,NULL))
                continue;
            atomEntryReq++;
        }
        //description RSS 2.0
        else if(xmlStrcmp(actItem->name,(xmlChar *)"description") == 0){
            if(loadValue(actItem,NULL))
                continue;
            rssItemReq++;
        }
    }
    //pokud splňuje item podmínky povinných elementů můžem načíst do výstupu
    if((atomEntryReq == 3 && atom == 1) || (rssItemReq >= 1 && rss == 1) || (rdfItemReq == 2 && rdf)) {
        printItem(title, time, author, email, url, tFlag, aFlag, uFlag);
        return 0;
    }
    return 1;
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
    int required = 0;
    for(xmlNode *actNode = node; actNode != NULL;actNode = actNode->next){
        if(xmlStrcmp(actNode->name,(xmlChar *)"channel") == 0) {
            for (xmlNode *actChannel = actNode->children; actChannel != NULL ; actChannel = actChannel->next) {
                //nadpis celého feedu (povinné)
                if(xmlStrcmp(actChannel->name,(xmlChar *)"title") == 0){
                    char *tmp;
                    if(loadValue(actChannel,&tmp)) {
                        fprintf(stderr,"Chyba, uzel neobshahuje hodnotu\n");
                        return 52;
                    }
                    sprintf(tmp,"*** %s ***",(char *)actChannel->children->content);
                    QueueUp(text,tmp);
                    free(tmp);
                    required++;
                }
                //url feedu (povinné)
                else if(xmlStrcmp(actChannel->name,(xmlChar *)"link") == 0){
                    if(loadValue(actChannel,NULL)) {
                        fprintf(stderr,"Chyba, uzel neobshahuje hodnotu\n");
                        return 52;
                    }
                    required++;
                }
                //popis feedu (povinné)
                else if(xmlStrcmp(actChannel->name,(xmlChar *)"description") == 0){
                    if(loadValue(actChannel,NULL)) {
                        fprintf(stderr,"Chyba, uzel neobshahuje hodnotu\n");
                        return 52;
                    }
                    required++;
                }
                //položky obsažené ve feedu (povinné)
                else if(xmlStrcmp(actChannel->name,(xmlChar *)"items") == 0){
                    required++;
                }
            }
        }
        //položka (item) povinnyý alespoň 1
        else if(xmlStrcmp(actNode->name,(xmlChar *)"item") == 0){
            if(required == 4)
                required++;
            if(processItem(actNode->children,tFlag,aFlag,uFlag)) {
                required = -255;
                break;
            }
        }

    }
    //pokud nemá povinné položky, chyba
    if(required != 5){
        fprintf(stderr,"Chyba: chybí povinné elementy pro formát RSS 1.0\n");
        return 53;
    }
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
    while(node != NULL && xmlStrcmp(node->name,(xmlChar *)"channel") != 0){
        node = node->next;
    }
    if(node == NULL){
        fprintf(stderr,"Chyba, nespravny format xml\n");
        return 52;
    }
    node = node->children;
    int required = 0;
    for(xmlNode *actNode = node; actNode != NULL;actNode = actNode->next){
        //nadpis celého feedu (povinné)
        if(xmlStrcmp(actNode->name,(xmlChar *)"title") == 0){
            char *tmp;
            if(loadValue(actNode,&tmp))
                return 52;
            sprintf(tmp,"*** %s ***",(char *)actNode->children->content);
            QueueUp(text,tmp);
            free(tmp);
            required++;
        }
        //url feedu (povinné)
        else if(xmlStrcmp(actNode->name,(xmlChar *)"link") == 0){
            if(loadValue(actNode,NULL)) {
                fprintf(stderr,"Chyba, uzel neobshahuje hodnotu\n");
                return 52;
            }
            required++;
        }
        //popis feedu (povinné)
        else if(xmlStrcmp(actNode->name,(xmlChar *)"description") == 0){
            if(loadValue(actNode,NULL)) {
                fprintf(stderr,"Chyba, uzel neobshahuje hodnotu\n");
                return 52;
            }
            required++;
        }
        //položka (item)
        else if(xmlStrcmp(actNode->name,(xmlChar *)"item") == 0){
            if(processItem(actNode->children,tFlag,aFlag,uFlag)) {
                required = -255;
                break;
            }
        }

    }
    //pokud nemá povinné položky, chyba
    if(required != 3){
        fprintf(stderr,"Chyba: chybí povinné elementy pro formát RSS 2.0\n");
        return 53;
    }
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

    int required = 0;
    for(xmlNode *actNode = node; actNode != NULL;actNode = actNode->next){
        //nadpis celého feedu (povinné)
        if(xmlStrcmp(actNode->name,(xmlChar *)"title") == 0){
            char *tmp;
            if(loadValue(actNode,&tmp))
                return 51;
            sprintf(tmp,"*** %s ***",(char *)actNode->children->content);
            QueueUp(text,tmp);
            free(tmp);
            required++;
        }
        //id feedu (povinné)
        else if(xmlStrcmp(actNode->name,(xmlChar *)"id") == 0){
            if(loadValue(actNode,NULL)) {
                fprintf(stderr,"Chyba, uzel neobshahuje hodnotu\n");
                return 52;
            }
            required++;
        }
        //čas aktualizace (povinné)
        else if(xmlStrcmp(actNode->name,(xmlChar *)"updated") == 0){
            if(loadValue(actNode,NULL)) {
                fprintf(stderr,"Chyba, uzel neobshahuje hodnotu\n");
                return 52;
            }
            required++;
        }
        //položka (entry)
        else if(xmlStrcmp(actNode->name,(xmlChar *)"entry") == 0){
            if(processItem(actNode->children,tFlag,aFlag,uFlag)){
                required = -255;
                break;
            }
        }

    }
    //pokud nemá povinné položky, chyba
    if(required != 3){
        fprintf(stderr,"Chyba: chybí povinné elementy pro formát Atom\n");
        return 53;
    }
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
    dc = 0;
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
        xmlFreeDoc(feed);
        xmlCleanupParser();
        return 51;
    }
    if(checkRoot(root)){
        printf("Chyba: kořen neobsahuje požadované atributy\n");
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
