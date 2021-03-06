/*
 * Copyright 2018 Vincent Sanders <vince@netsurf-browser.org>
 *
 * This file is part of libnspdf.
 *
 * Licensed under the MIT License,
 *                http://www.opensource.org/licenses/mit-license.php
 */

#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>

#include <nspdf/errors.h>

#include "cos_parse.h"
#include "cos_object.h"
#include "pdf_doc.h"
#include "xref.h"


/** indirect object */
struct xref_table_entry {
    /* reference identifier */
    struct cos_reference ref;

    /** offset of object */
    strmoff_t offset;

    /* indirect object if already decoded */
    struct cos_object *object;
};

static struct cos_object cos_null_obj = {
    .type = COS_TYPE_NULL,
};

nspdferror nspdf__xref_allocate(struct nspdf_doc *doc, int64_t size)
{
    if (doc->xref_table != NULL) {
        /** \todo handle freeing xref table */
        return NSPDFERROR_SYNTAX;
    }
    doc->xref_table_size = size;

    doc->xref_table = calloc(doc->xref_table_size,
                             sizeof(struct xref_table_entry));
    if (doc->xref_table == NULL) {
        return NSPDFERROR_NOMEM;
    }
    return NSPDFERROR_OK;
}

nspdferror
nspdf__xref_parse(struct nspdf_doc *doc,
                  struct cos_stream *stream,
                  strmoff_t *offset_out)
{
    strmoff_t offset;
    nspdferror res;
    uint64_t objnumber; /* current object number */
    uint64_t objcount;

    offset = *offset_out;

    /* xref object header */
    if ((stream_byte(stream, offset    ) != 'x') ||
        (stream_byte(stream, offset + 1) != 'r') ||
        (stream_byte(stream, offset + 2) != 'e') ||
        (stream_byte(stream, offset + 3) != 'f')) {
        return NSPDFERROR_SYNTAX;
    }
    offset += 4;

    res = nspdf__stream_skip_ws(stream, &offset);
    if (res != NSPDFERROR_OK) {
        return res;
    }

    /* subsections
     * <first object number> <number of references in subsection>
     */
    res = nspdf__stream_read_uint(stream, &offset, &objnumber);
    while (res == NSPDFERROR_OK) {
        uint64_t lastobj;
        res = nspdf__stream_skip_ws(stream, &offset);
        if (res != NSPDFERROR_OK) {
            return res;
        }

        res = nspdf__stream_read_uint(stream, &offset, &objcount);
        if (res != NSPDFERROR_OK) {
            return res;
        }

        res = nspdf__stream_skip_ws(stream, &offset);
        if (res != NSPDFERROR_OK) {
            return res;
        }

        //printf("decoding subsection %lld %lld\n", objnumber, objcount);

        lastobj = objnumber + objcount;
        for (; objnumber < lastobj ; objnumber++) {
            /* each entry is a fixed format */
            uint64_t objindex;
            uint64_t objgeneration;

            /* object index */
            res = nspdf__stream_read_uint(stream, &offset, &objindex);
            if (res != NSPDFERROR_OK) {
                return res;
            }
            offset++; /* skip space */

            res = nspdf__stream_read_uint(stream, &offset, &objgeneration);
            if (res != NSPDFERROR_OK) {
                return res;
            }
            offset++; /* skip space */

            if ((stream_byte(stream, offset++) == 'n')) {
                if (objnumber < doc->xref_table_size) {
                    struct xref_table_entry *indobj;
                    indobj = doc->xref_table + objnumber;

                    indobj->ref.id = objnumber;
                    indobj->ref.generation = objgeneration;
                    indobj->offset = objindex;

                    //printf("xref %lld %lld -> %lld\n", objnumber, objgeneration, objindex);
                } else {
                    //printf("index out of bounds\n");
                }
            }

            offset += 2; /* skip EOL */
        }

        res = nspdf__stream_read_uint(stream, &offset, &objnumber);
    }

    return NSPDFERROR_OK;
}


nspdferror
nspdf__xref_get_referenced(struct nspdf_doc *doc, struct cos_object **cobj_out)
{
    nspdferror res;
    struct cos_object *cobj;
    struct cos_object *indirect;
    strmoff_t offset;
    struct xref_table_entry *entry;

    cobj = *cobj_out;

    if (cobj->type != COS_TYPE_REFERENCE) {
        /* not passed a reference object so just return what was passed */
        return NSPDFERROR_OK;
    }

    if (doc == NULL) {
        /* a reference with no document to dereference against */
        return NSPDFERROR_REFERENCE;
    }

    entry = doc->xref_table + cobj->u.reference->id;

    /* check if referenced object is in range and exists. return null object if
     * not
     */
    if ((cobj->u.reference->id >= doc->xref_table_size) ||
        (cobj->u.reference->id == 0) ||
        (entry->ref.id == 0)) {
        *cobj_out = &cos_null_obj;
        return NSPDFERROR_OK;
    }

    if (entry->object == NULL) {
        /* indirect object has never been parsed */
        offset = entry->offset;
        res = cos_parse_object(doc, doc->stream, &offset, &indirect);
        if (res != NSPDFERROR_OK) {
            //printf("failed to decode indirect object\n");
            return res;
        }

        entry->object = indirect;
    }

    *cobj_out = entry->object;

    return NSPDFERROR_OK;
}
