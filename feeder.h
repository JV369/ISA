#include <stdio.h>
#include "argChecker.h"
#include <openssl/ssl.h>
#include <openssl/bio.h>
#include <openssl/err.h>
#include <openssl/evp.h>
#include <libxml2/libxml/tree.h>
#include <libxml2/libxml/parser.h>
#include <openssl/conf.h>


int feedreader(TQueue *url, TQueue *cert, int certFlag, int tFlag, int aFlag, int uFlag);