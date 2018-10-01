/**
 * Hlavičkový soubor pro frontu
 * Autor: Jan Vávra
 * Login: xvavra20
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/**
 * Fronta pro uložení elementů během měření
 */
typedef struct QElem{
    char *str;
    struct QElem *next;
} *TQElem;

typedef struct Queue{
    TQElem front;
    TQElem end;
} TQueue;

/**
 * inicializuje frontu pro element QElem
 * @param queue fronta, kterou chceme inicializovat
 */
void QueueInit(TQueue *queue);
/**
 * přidá element do fronty
 * @param queue fronta, do které chceme prvek přidat
 * @param str řetězec
 */
void QueueUp(TQueue *queue, char *str);
/**
 * vrátí element ze začátku fronty a odstraní ho ze začátku
 * @param queue fronta, ze které chceme výjmout element
 * @param message ukazatel na proměnou do které se má uložit řetězec
 */
void QueueFrontPop(TQueue *queue, char **str);
/**
 * uvolní všechny elementy z fronty
 * @param queue fronta, která se má uvolnit (pro elementy QElem)
 */
void QueueDestroy(TQueue *queue);
