/*
 * Copyright 2018 Vincent Sanders <vince@netsurf-browser.org>
 *
 * This file is part of libnspdf.
 *
 * Licensed under the MIT License,
 *                http://www.opensource.org/licenses/mit-license.php
 */

/**
 * \file
 * NetSurf PDF library cross reference table handling
 */

#ifndef NSPDF__XREF_H_
#define NSPDF__XREF_H_

#include "cos_stream.h"

struct nspdf_doc;
struct cos_object;

/**
 * parse xref from file
 */
nspdferror nspdf__xref_parse(struct nspdf_doc *doc, struct cos_stream *stream, strmoff_t *offset_out);


/**
 * get an object dereferencing through xref table if necessary
 */
nspdferror nspdf__xref_get_referenced(struct nspdf_doc *doc, struct cos_object **cobj_out);

/**
 * allocate storage for cross reference table
 */
nspdferror nspdf__xref_allocate(struct nspdf_doc *doc, int64_t size);

#endif
