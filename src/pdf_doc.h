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
 * NetSurf PDF library pdf document
 */

#ifndef NSPDF__PDF_DOC_H_
#define NSPDF__PDF_DOC_H_

#include "cos_stream.h"

struct xref_table_entry;
struct page_table_entry;

/**
 * pdf document
 */
struct nspdf_doc {

    const uint8_t *start; /* start of pdf document in input stream */
    unsigned int length;

    /**
     * input data stream
     */
    struct cos_stream *stream;

    int major;
    int minor;

    /**
     * Indirect object cross reference table
     */
    uint64_t xref_table_size;
    struct xref_table_entry *xref_table;

    struct cos_object *root;
    struct cos_object *encrypt;
    struct cos_object *info;
    struct cos_object *id;

    /* page refrerence table */
    uint64_t page_table_size;
    struct page_table_entry *page_table;
};

/* helpers in pdf_doc.c */
nspdferror nspdf__stream_skip_ws(struct cos_stream *stream, strmoff_t *offset);
nspdferror nspdf__stream_skip_eol(struct cos_stream *stream, strmoff_t *offset);
nspdferror nspdf__stream_read_uint(struct cos_stream *stream, strmoff_t *offset_out, uint64_t *result_out);


nspdferror nspdf__decode_page_tree(struct nspdf_doc *doc, struct cos_object *page_tree_node, unsigned int *page_index);

/* cos stream filters */
nspdferror nspdf__cos_stream_filter(struct nspdf_doc *doc, const char *filter_name, struct cos_stream **stream_out);

#endif
