/**
 * Hlavičkový soubor - Parser pro feedy Atom a RSS
 * Autor: Jan Vávra
 * Login: xvavra20
 */

#include <stdio.h>
#include "argChecker.h"
#include <libxml/tree.h>
#include <libxml/parser.h>

/**
 * Vstupní bod do parseru xml
 * @param input - vstupní xml
 * @param tFLag - flag, jestli máme zahrnout čas do výstupu
 * @param aFLag - flag, jestli máme zahrnout autora do výstupu
 * @param uFLag - flag, jestli máme zahrnout URL do výstupu
 * @return 0 pokud je vše v pořádku,
 *         51 při chybě v načítání nebo v kořeni xml,
 *         52 při chybě v těle před item/entry,
 *         53 při chybě uvnitř item/entry
 */
int parsexml(char *input, int tFlag, int aFlag, int uFlag);
