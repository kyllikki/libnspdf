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

/**
 * move offset to next non whitespace byte
 */
nspdferror doc_skip_ws(struct nspdf_doc *doc, uint64_t *offset)
{
    uint8_t c;
    /* TODO sort out keeping offset in range */
    c = DOC_BYTE(doc, *offset);
    while ((bclass[c] & (BC_WSPC | BC_CMNT) ) != 0) {
        (*offset)++;
        /* skip comments */
        if ((bclass[c] & BC_CMNT) != 0) {
            c = DOC_BYTE(doc, *offset);
            while ((bclass[c] & BC_EOLM ) == 0) {
                (*offset)++;
                c = DOC_BYTE(doc, *offset);
            }
        }
        c = DOC_BYTE(doc, *offset);
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

static struct cos_object cos_null_obj = {
    .type = COS_TYPE_NULL,
};

nspdferror
xref_get_referenced(struct nspdf_doc *doc, struct cos_object **cobj_out)
{
    nspdferror res;
    struct cos_object *cobj;
    struct cos_object *indirect;
    uint64_t offset;
    struct xref_table_entry *entry;

    cobj = *cobj_out;

    if (cobj->type != COS_TYPE_REFERENCE) {
        /* not passed a reference object so just return what was passed */
        return NSPDFERROR_OK;
    }

    entry = doc->xref_table + cobj->u.reference->id;

    /* check if referenced object is in range and exists. return null object if
     * not
     */
    if ((cobj->u.reference->id >= doc->xref_size) ||
        (cobj->u.reference->id == 0) ||
        (entry->ref.id == 0)) {
        *cobj_out = &cos_null_obj;
        return NSPDFERROR_OK;
    }

    if (entry->object == NULL) {
        /* indirect object has never been decoded */
        offset = entry->offset;
        res = cos_parse_object(doc, &offset, &indirect);
        if (res != NSPDFERROR_OK) {
            printf("failed to decode indirect object\n");
            return res;
        }

        entry->object = indirect;
    }

    *cobj_out = entry->object;

    return NSPDFERROR_OK;
}
