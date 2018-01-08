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

#include <nspdf/errors.h>

#include "cos_object.h"
#include "pdf_doc.h"

nspdferror
nspdf__cos_stream_filter(struct nspdf_doc *doc,
                         const char *filter_name,
                         struct cos_stream **stream_out)
{
    struct cos_stream *stream_in;

    stream_in = *stream_out;

    printf("applying filter %s\n", filter_name);
    return NSPDFERROR_OK;
}
