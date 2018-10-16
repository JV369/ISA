#include "feeder.h"

int loadValue(xmlNode *node, char **str){
    if(node->children == NULL && xmlStrcmp(node->name,(xmlChar *)"text") == 0
       && node->children->next != NULL && node->children->content == NULL){
        fprintf(stderr,"Chyba, nespravny format xml\n");
        return 1;
    }
    if(str == NULL){
        return 0;
    }
    *str = (char *)malloc(strlen((char *)node->children->content)+1);
    strcpy(*str,(char *)node->children->content);
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

int rdfProcessItem(xmlNode *rssItem, int tFlag, int aFlag, int uFlag){
    char *title = NULL;
    char *time = NULL;
    char *author = NULL;
    char *url = NULL;

    for (xmlNode *actItem = rssItem; actItem != NULL ; actItem = actItem->next) {
        //printf("%s\n",actItem->name);
        if(xmlStrcmp(actItem->name,(xmlChar *)"title") == 0){
            if(loadValue(actItem,&title))
                continue;
        }
        else if(xmlStrcmp(actItem->name,(xmlChar *)"link") == 0){
            if(loadValue(actItem,&url))
                continue;
        }
        else if(xmlStrcmp(actItem->name,(xmlChar *)"creator") == 0){
            if(xmlStrcmp(actItem->children->name,(xmlChar *)"name") == 0){
                for (xmlNode *tmp = actItem->children; tmp != NULL; tmp = tmp->next) {
                    if(xmlStrcmp(tmp->name,(xmlChar *)"name") == 0){
                        char *firstName = NULL;
                        char *middleName = NULL;
                        char *lastName = NULL;
                        for (xmlNode *authorName = tmp->children;authorName != NULL;authorName = authorName->next) {
                            if(xmlStrcmp(authorName->name,(xmlChar *)"firstname") == 0){
                                firstName = (char *)authorName->children->content;
                            }
                            else if(xmlStrcmp(authorName->name,(xmlChar *)"middle_initial") == 0){
                                middleName = (char *)authorName->children->content;
                            }
                            else if(xmlStrcmp(authorName->name,(xmlChar *)"middle_initial") == 0){
                                lastName = (char *)authorName->children->content;
                            }
                        }
                        int first = 1;
                        if(firstName != NULL){
                            first = 0;
                            author = (char *)malloc(strlen(firstName)+1);
                            strcpy(author,firstName);
                            strcat(author," ");
                        }
                        if(middleName != NULL){
                            if(first){
                                first = 0;
                                author = (char *)malloc(strlen(middleName)+1);
                                strcpy(author,middleName);
                                strcat(author," ");
                            }
                            else{
                                author = (char *)realloc(author,strlen(author)+strlen(middleName));
                                strcat(author,middleName);
                                strcat(author," ");
                            }
                        }
                        if(lastName != NULL){
                            if(first){
                                author = (char *)malloc(strlen(lastName)+1);
                                strcpy(author,lastName);
                            }
                            else{
                                author = (char *)realloc(author,strlen(author)+strlen(lastName));
                                strcat(author,lastName);
                            }
                        }
                    }
                }
                continue;
            }
            author = (char *)malloc(strlen((char *)actItem->children->content)+1);
            strcpy(author,(char *)actItem->children->content);
        }
    }

    printItem(title,time,author,NULL,url,tFlag,aFlag,uFlag);
    return 0;
}

int parseRdf(xmlNode *node, int tFlag, int aFlag, int uFlag){
    if(node == NULL){
        fprintf(stderr,"Chyba, nespravny format xml\n");
        return 51;
    }
    for(xmlNode *actNode = node; actNode != NULL;actNode = actNode->next){
        if(actNode->name == NULL){
            fprintf(stderr,"Chyba, nespravny format xml\n");
            return 51;
        }
        else if(xmlStrcmp(actNode->name,(xmlChar *)"channel") == 0) {
            for (xmlNode *actChannel = actNode->children; actChannel != NULL ; actChannel = actChannel->next) {
                if(actChannel->name == NULL){
                    fprintf(stderr,"Chyba, nespravny format xml\n");
                    return 51;
                }
                else if(xmlStrcmp(actChannel->name,(xmlChar *)"title") == 0){
                    if(loadValue(actChannel,NULL))
                        return 51;
                    printf("*** %s ***\n",actChannel->children->content);
                    break;
                }
            }
        }
        else if(xmlStrcmp(actNode->name,(xmlChar *)"item") == 0){
            rdfProcessItem(actNode->children,tFlag,aFlag,uFlag);
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
        if(xmlStrcmp(actItem->name,(xmlChar *)"title") == 0){
            if(loadValue(actItem,&title))
                continue;
        }
        else if(xmlStrcmp(actItem->name,(xmlChar *)"pubDate") == 0){
            if(loadValue(actItem,&time))
                continue;
        }
        else if(xmlStrcmp(actItem->name,(xmlChar *)"link") == 0){
            if(loadValue(actItem,&url))
                continue;
        }
        else if(xmlStrcmp(actItem->name,(xmlChar *)"guid") == 0){
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
        else if(xmlStrcmp(actItem->name,(xmlChar *)"author") == 0){
            if(loadValue(actItem,&author))
                continue;
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
        if(actNode->name == NULL){
            fprintf(stderr,"Chyba, nespravny format xml\n");
            return 1;
        }
        else if(xmlStrcmp(actNode->name,(xmlChar *)"title") == 0){
            if(loadValue(actNode,NULL))
                return 51;
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
            if(loadValue(actItem,&title))
                continue;
        }
        else if(xmlStrcmp(actItem->name,(xmlChar *)"updated") == 0){
            if(loadValue(actItem,&time))
                continue;
        }
        else if(xmlStrcmp(actItem->name,(xmlChar *)"link") == 0){
            if(actItem->properties == NULL){
                fprintf(stderr,"Chyba, nespravny format xml\n");
                continue;
            }
            for (xmlAttr *actUrl = actItem->properties; actUrl != NULL ; actUrl = actUrl->next) {
                if(xmlStrcmp(actUrl->name,(xmlChar *)"href") == 0){
                    if(loadValue((xmlNode *)actUrl,&url))
                        continue;
                }
            }
        }
        else if(xmlStrcmp(actItem->name,(xmlChar *)"author") == 0){
            for (xmlNode *actAuthor = actItem->children; actAuthor != NULL; actAuthor = actAuthor->next) {
                if(xmlStrcmp(actAuthor->name,(xmlChar *)"name") == 0){
                    if(loadValue(actAuthor,&author))
                        continue;
                }
                else if(xmlStrcmp(actAuthor->name,(xmlChar *)"email") == 0){
                    if(loadValue(actAuthor,&email))
                        continue;
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
        if(actNode->name == NULL){
            fprintf(stderr,"Chyba, nespravny format xml\n");
            return 1;
        }
        else if(xmlStrcmp(actNode->name,(xmlChar *)"title") == 0){
            if(loadValue(actNode,NULL))
                return 51;
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

    feed = xmlReadDoc((xmlChar *)input,"newurl.org",NULL,0);

    root = xmlDocGetRootElement(feed);
    if(root == NULL){
        fprintf(stderr,"Chyba při načítání xml\n");
        xmlFreeDoc(feed);
        xmlCleanupParser();
        return 52;
    }
    else if(xmlStrcmp(root->name,(xmlChar *)"rss") == 0){
        parseRss(root->children, tFlag,aFlag,uFlag);
    }
    else if(xmlStrcmp(root->name,(xmlChar *)"feed") == 0){
        parseAtom(root->children, tFlag,aFlag,uFlag);
    }
    else if(xmlStrcmp(root->name,(xmlChar *)"RDF") == 0){
        parseRdf(root->children, tFlag,aFlag,uFlag);
    }
    else{
        fprintf(stderr,"Chyba při rozpoznávání xml templatu\n");
        return 51;
    }
    xmlFreeDoc(feed);
    xmlCleanupParser();
    return 0;
}
