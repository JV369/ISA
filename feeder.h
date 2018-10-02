#include <stdio.h>
#include "argChecker.h"
#include "openssl/ssl.h"
#include "openssl/bio.h"
#include "openssl/err.h"

int feedreader(TQueue *url, TQueue *cert, int certFlag, int tFlag, int aFlag, int uFlag);