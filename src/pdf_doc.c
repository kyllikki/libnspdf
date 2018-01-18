/*
 * Copyright 2017 Vincent Sanders <vince@netsurf-browser.org>
 *
 * This file is part of libnspsl
 *
 * Licensed under the MIT License,
 *                http://www.opensource.org/licenses/mit-license.php
 */

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdio.h>

#include <nspdf/errors.h>

#include "cos_parse.h"
#include "byte_class.h"
#include "cos_object.h"
#include "pdf_doc.h"

nspdferror nspdf__stream_skip_ws(struct cos_stream *stream, uint64_t *offset)
{
    uint8_t c;
    /* TODO sort out keeping offset in range */
    c = stream_byte(stream, *offset);
    while ((bclass[c] & (BC_WSPC | BC_CMNT) ) != 0) {
        (*offset)++;
        /* skip comments */
        if ((bclass[c] & BC_CMNT) != 0) {
            c = stream_byte(stream, *offset);
            while ((bclass[c] & BC_EOLM ) == 0) {
                (*offset)++;
                c = stream_byte(stream, *offset);
            }
        }
        c = stream_byte(stream, *offset);
    }
    return NSPDFERROR_OK;
}


/**
 * move offset to next non eol byte
 */
nspdferror doc_skip_eol(struct nspdf_doc *doc, uint64_t *offset)
{
    uint8_t c;
    /* TODO sort out keeping offset in range */
    c = DOC_BYTE(doc, *offset);
    while ((bclass[c] & BC_EOLM) != 0) {
        (*offset)++;
        c = DOC_BYTE(doc, *offset);
    }
    return NSPDFERROR_OK;
}


nspdferror
doc_read_uint(struct nspdf_doc *doc,
              uint64_t *offset_out,
              uint64_t *result_out)
{
    uint8_t c; /* current byte from source data */
    unsigned int len; /* number of decimal places in number */
    uint8_t num[21]; /* temporary buffer for decimal values */
    uint64_t offset; /* current offset of source data */
    uint64_t result=0; /* parsed result */
    uint64_t tens;

    offset = *offset_out;

    for (len = 0; len < sizeof(num); len++) {
        c = DOC_BYTE(doc, offset);
        if ((bclass[c] & BC_DCML) != BC_DCML) {
            if (len == 0) {
                return -2; /* parse error no decimals in input */
            }
            /* sum value from each place */
            for (tens = 1; len > 0; tens = tens * 10, len--) {
                result += (num[len - 1] * tens);
            }

            *offset_out = offset;
            *result_out = result;

            return NSPDFERROR_OK;
        }
        num[len] = c - '0';
        offset++;
    }
    return -1; /* number too long */
}
