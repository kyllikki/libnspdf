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

struct xref_table_entry;
struct page_table_entry;

/** pdf document */
struct nspdf_doc {

    const uint8_t *start; /* start of pdf document in input stream */
    uint64_t length;

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

/* byte data acessory, allows for more complex buffer handling in future */
#define DOC_BYTE(doc, offset) (doc->start[(offset)])

/* helpers in pdf_doc.h */
nspdferror doc_skip_ws(struct nspdf_doc *doc, uint64_t *offset);
nspdferror doc_skip_eol(struct nspdf_doc *doc, uint64_t *offset);
nspdferror doc_read_uint(struct nspdf_doc *doc, uint64_t *offset_out, uint64_t *result_out);

/**
 * parse xref from file
 */
nspdferror nspdf__xref_parse(struct nspdf_doc *doc, uint64_t *offset_out);

/**
 * get an object dereferencing through xref table if necessary
 */
nspdferror nspdf__xref_get_referenced(struct nspdf_doc *doc, struct cos_object **cobj_out);

nspdferror nspdf__xref_allocate(struct nspdf_doc *doc, int64_t size);

nspdferror nspdf__decode_page_tree(struct nspdf_doc *doc, struct cos_object *page_tree_node, unsigned int *page_index);

#endif
