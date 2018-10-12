//
// Created by jan on 12.10.18.
//

#include <stdio.h>
#include <openssl/ssl.h>
#include <openssl/bio.h>
#include <openssl/err.h>
#include <openssl/evp.h>
#include <openssl/conf.h>
#include "feeder.h"

int feedreader(TQueue *url, char *certFile, char *certAddr, int tFlag, int aFlag, int uFlag);