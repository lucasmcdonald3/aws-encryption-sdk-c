/* 
 * Copyright 2018 Amazon.com, Inc. or its affiliates. All Rights Reserved.
 * 
 * Licensed under the Apache License, Version 2.0 (the "License"). You may not use
 * this file except in compliance with the License. A copy of the License is
 * located at
 * 
 *     http://aws.amazon.com/apache2.0/
 * 
 * or in the "license" file accompanying this file. This file is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or
 * implied. See the License for the specific language governing permissions and
 * limitations under the License.
 */ 

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include "testutil.h"

int test_loadfile(const char *filename, uint8_t **buf, size_t *datasize) {
    uint8_t *tmpbuf = NULL;
    FILE *fp = fopen(filename, "rb");

    if (!fp) {
        return 1;
    }

    size_t bufsz = 128;
    size_t offset = 0;

    tmpbuf = malloc(bufsz);

    if (!tmpbuf) {
        fclose(fp);
        errno = ENOMEM;
        return 1;
    }

    while (!feof(fp)) {
        if (offset == bufsz) {
            size_t newsz = bufsz * 2;
            if (newsz <= bufsz) {
                errno = ENOMEM;
                goto failure;
            }

            uint8_t *newptr = realloc(tmpbuf, newsz);
            if (!newptr) {
                errno = ENOMEM;
                goto failure;
            }

            tmpbuf = newptr;
            bufsz = newsz;
        }

        size_t nread = fread(tmpbuf + offset, 1, bufsz - offset, fp);
        if (ferror(fp)) {
            errno = EIO;
            goto failure;
        }

        offset += nread;
    }

    {
        *buf = realloc(tmpbuf, offset);
        if (!*buf) {
            errno = ENOMEM;
            goto failure;
        }

        *datasize = offset;
        return 0;
    }

failure:
    /*
     *  we need this semicolon as we can't have a variable declaration right
     * after a label
     */
    ;

    int saved_errno = errno;

    if (tmpbuf) free(tmpbuf);
    if (fp) fclose(fp);

    errno = saved_errno;

    return 1;
}
