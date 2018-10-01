/**
 * Kod pro frontu
 * Autor: Jan Vávra
 * Login: xvavra20
 */


#include "queue.h"


void QueueInit(TQueue *queue){
    queue->front = NULL;
    queue->end = NULL;
}

void QueueUp(TQueue *queue,char *str){
    TQElem newElem = (TQElem)malloc(sizeof(struct QElem));

    newElem->str = (char *)malloc(strlen(str)+1);
    strcpy(newElem->str,str);
    newElem->next = NULL;
    if(queue->front != NULL){
        queue->end->next = newElem;
        queue->end = newElem;
    }
    else{
        queue->front = newElem;
        queue->end = newElem;
    }
}


void QueueFrontPop(TQueue *queue, char **str){
    TQElem temp = queue->front;
    if(queue->front != queue->end)
        queue->front = queue->front->next;
    else{
        queue->front = NULL;
        queue->end = NULL;
    }

    *str = (char *)malloc(strlen(temp->str)+1);
    *str = strcpy(*str,temp->str);

    free(temp->str);
    free(temp);
}



void QueueDestroy(TQueue *queue){
    queue->end = NULL;
    while(queue->front != NULL){
        TQElem temp = queue->front;
        queue->front = queue->front->next;
        free(temp->str);
        free(temp);
    }
}
