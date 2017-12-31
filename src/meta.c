/*
 * Copyright 2018 Vincent Sanders <vince@netsurf-browser.org>
 *
 * This file is part of libnspdf.
 *
 * Licensed under the MIT License,
 *                http://www.opensource.org/licenses/mit-license.php
 */

#include <libwapcaplet/libwapcaplet.h>

#include <nspdf/meta.h>

#include "cos_object.h"
#include "pdf_doc.h"

static nspdferror lwc2nspdferr(lwc_error ret)
{
    nspdferror res;

    switch (ret) {
    case lwc_error_ok:
        res = NSPDFERROR_OK;
        break;

    case lwc_error_oom:
        res = NSPDFERROR_NOMEM;
        break;

    case lwc_error_range:
        res = NSPDFERROR_RANGE;
        break;

    default:
        res = NSPDFERROR_NOTFOUND;
        break;
    }
    return res;
}

nspdferror nspdf_get_title(struct nspdf_doc *doc, struct lwc_string_s **title)
{
    struct cos_string *cos_title;
    nspdferror res;

    if (doc->info == NULL) {
        return NSPDFERROR_NOTFOUND;
    }

    res = cos_get_dictionary_string(doc, doc->info, "Title", &cos_title);
    if (res != NSPDFERROR_OK) {
        return res;
    }

    res = lwc2nspdferr(lwc_intern_string((const char *)cos_title->data,
                                         cos_title->length,
                                         title));

    return res;
}
