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

nspdferror
nspdf__stream_skip_ws(struct cos_stream *stream, strmoff_t *offset)
{
    uint8_t c;

    if ((*offset) >= stream->length) {
        return NSPDFERROR_OK;
    }

    c = stream_byte(stream, *offset);
    while (((*offset) < stream->length) &&
           ((bclass[c] & (BC_WSPC | BC_CMNT) ) != 0)) {
        (*offset)++;
        /* skip comments */
        if (((*offset) < stream->length) &&
            ((bclass[c] & BC_CMNT) != 0)) {
            c = stream_byte(stream, *offset);
            while ((*offset < stream->length) &&
                   ((bclass[c] & BC_EOLM ) == 0)) {
                (*offset)++;
                c = stream_byte(stream, (*offset));
            }
        }
        c = stream_byte(stream, (*offset));
    }
    return NSPDFERROR_OK;
}


/**
 * move offset to next non eol byte
 */
nspdferror
nspdf__stream_skip_eol(struct cos_stream *stream, strmoff_t *offset)
{
    uint8_t c;
    /** \todo sort out keeping offset in range */
    c = stream_byte(stream, *offset);
    while ((bclass[c] & BC_EOLM) != 0) {
        (*offset)++;
        c = stream_byte(stream, *offset);
    }
    return NSPDFERROR_OK;
}


nspdferror
nspdf__stream_read_uint(struct cos_stream *stream,
                        strmoff_t *offset_out,
                        uint64_t *result_out)
{
    uint8_t c; /* current byte from source data */
    strmoff_t offset; /* current offset of source data */
    unsigned int len; /* number of decimal places in number */
    uint8_t num[21]; /* temporary buffer for decimal values */
    uint64_t result=0; /* parsed result */
    uint64_t tens;

    offset = *offset_out;

    for (len = 0; len < sizeof(num); len++) {
        c = stream_byte(stream, offset);
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
    return NSPDFERROR_RANGE; /* number too long */
}
