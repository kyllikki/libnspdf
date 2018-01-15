/*
 * Copyright 2018 Vincent Sanders <vince@netsurf-browser.org>
 *
 * This file is part of libnspdf.
 *
 * Licensed under the MIT License,
 *                http://www.opensource.org/licenses/mit-license.php
 */

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <zlib.h>

#include <nspdf/errors.h>

#include "cos_object.h"
#include "pdf_doc.h"

static nspdferror
cos_stream_inflate(struct nspdf_doc *doc, struct cos_stream **stream_out)
{
    int ret;
    z_stream strm;
    struct cos_stream *stream_in;
    struct cos_stream *stream_res;

    stream_in = *stream_out;

    stream_res = calloc(1, sizeof(struct cos_stream));

    //printf("inflating from %d bytes\n", stream_in->length);

    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;
    strm.avail_in = 0;
    strm.next_in = Z_NULL;

    ret = inflateInit(&strm);
    if (ret != Z_OK) {
        return NSPDFERROR_NOTFOUND;
    }

    strm.next_in = (void *)stream_in->data;
    strm.avail_in = stream_in->length;

    do {
        int64_t available; /* available buffer space for decompression */
        available = stream_res->alloc - stream_res->length;

        if (available < (strm.avail_in << 1)) {
            uint8_t *newdata;
            size_t newlength;

            newlength = stream_res->alloc + (stream_in->length << 1);
            newdata = realloc((void *)stream_res->data, newlength);
            if (newdata == NULL) {
                free((void *)stream_res->data);
                free(stream_res);
                inflateEnd(&strm);
                return NSPDFERROR_NOMEM;
            }

            //printf("realloc %d\n", newlength);

            stream_res->data = newdata;
            stream_res->alloc = newlength;
            available = stream_res->alloc - stream_res->length;
        }

        strm.avail_out = available;
        strm.next_out = (void*)(stream_res->data + stream_res->length);
        ret = inflate(&strm, Z_NO_FLUSH);
        /** \todo check zlib return code */

        stream_res->length += (available - strm.avail_out);

    } while (ret != Z_STREAM_END);

    //printf("allocated %d\n", stream_res->alloc);

    //printf("length %d\n", stream_res->length);

    inflateEnd(&strm);

    if (stream_in->alloc != 0) {
        free((void*)stream_in->data);
    }
    free(stream_in);

    *stream_out = stream_res;

    return NSPDFERROR_OK;
}

nspdferror
nspdf__cos_stream_filter(struct nspdf_doc *doc,
                         const char *filter_name,
                         struct cos_stream **stream_out)
{
    nspdferror res;

    //printf("applying filter %s\n", filter_name);

    if (strcmp(filter_name, "FlateDecode") == 0) {
        res = cos_stream_inflate(doc, stream_out);
    } else {
        res = NSPDFERROR_NOTFOUND;
    }

    return res;
}
