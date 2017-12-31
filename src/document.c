/*
 * Copyright 2018 Vincent Sanders <vince@netsurf-browser.org>
 *
 * This file is part of libnspdf.
 *
 * Licensed under the MIT License,
 *                http://www.opensource.org/licenses/mit-license.php
 */

#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include <nspdf/document.h>

#include "cos_parse.h"
#include "byte_class.h"
#include "cos_object.h"
#include "pdf_doc.h"

#define SLEN(x) (sizeof((x)) - 1)

#define STARTXREF_TOK "startxref"

/* Number of bytes to search back from file end to find xref start token,
 * convention says 1024 bytes
 */
#define STARTXREF_SEARCH_SIZE 1024


static nspdferror
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


/**
 * finds the startxref marker at the end of input
 */
static nspdferror find_startxref(struct nspdf_doc *doc, uint64_t *offset_out)
{
    uint64_t offset; /* offset of characters being considered for startxref */
    uint64_t earliest; /* earliest offset to serch for startxref */

    offset = doc->length - SLEN(STARTXREF_TOK);

    if (doc->length < STARTXREF_SEARCH_SIZE) {
        earliest = 0;
    } else {
        earliest = doc->length - STARTXREF_SEARCH_SIZE;
    }

    for (;offset > earliest; offset--) {
        if ((DOC_BYTE(doc, offset    ) == 's') &&
            (DOC_BYTE(doc, offset + 1) == 't') &&
            (DOC_BYTE(doc, offset + 2) == 'a') &&
            (DOC_BYTE(doc, offset + 3) == 'r') &&
            (DOC_BYTE(doc, offset + 4) == 't') &&
            (DOC_BYTE(doc, offset + 5) == 'x') &&
            (DOC_BYTE(doc, offset + 6) == 'r') &&
            (DOC_BYTE(doc, offset + 7) == 'e') &&
            (DOC_BYTE(doc, offset + 8) == 'f')) {
            *offset_out = offset;
            return NSPDFERROR_OK;
        }
    }
    return NSPDFERROR_SYNTAX;
}


/**
 * decodes a startxref field
 */
static nspdferror
decode_startxref(struct nspdf_doc *doc,
                 uint64_t *offset_out,
                 uint64_t *start_xref_out)
{
    uint64_t offset; /* offset of characters being considered for startxref */
    uint64_t start_xref;
    nspdferror res;

    offset = *offset_out;

    if ((DOC_BYTE(doc, offset    ) != 's') ||
        (DOC_BYTE(doc, offset + 1) != 't') ||
        (DOC_BYTE(doc, offset + 2) != 'a') ||
        (DOC_BYTE(doc, offset + 3) != 'r') ||
        (DOC_BYTE(doc, offset + 4) != 't') ||
        (DOC_BYTE(doc, offset + 5) != 'x') ||
        (DOC_BYTE(doc, offset + 6) != 'r') ||
        (DOC_BYTE(doc, offset + 7) != 'e') ||
        (DOC_BYTE(doc, offset + 8) != 'f')) {
        return NSPDFERROR_SYNTAX;
    }
    offset += 9;

    res = doc_skip_ws(doc, &offset);
    if (res != NSPDFERROR_OK) {
        return res;
    }

    res = doc_read_uint(doc, &offset, &start_xref);
    if (res != NSPDFERROR_OK) {
        return res;
    }

    res = doc_skip_eol(doc, &offset);
    if (res != NSPDFERROR_OK) {
        return res;
    }

    if ((DOC_BYTE(doc, offset    ) != '%') ||
        (DOC_BYTE(doc, offset + 1) != '%') ||
        (DOC_BYTE(doc, offset + 2) != 'E') ||
        (DOC_BYTE(doc, offset + 3) != 'O') ||
        (DOC_BYTE(doc, offset + 4) != 'F')) {
        printf("missing EOF marker\n");
        return NSPDFERROR_SYNTAX;
    }

    *offset_out = offset;
    *start_xref_out = start_xref;

    return NSPDFERROR_OK;
}


/**
 * finds the next trailer
 */
static nspdferror find_trailer(struct nspdf_doc *doc, uint64_t *offset_out)
{
    uint64_t offset; /* offset of characters being considered for trailer */

    for (offset = *offset_out;offset < doc->length; offset++) {
        if ((DOC_BYTE(doc, offset    ) == 't') &&
            (DOC_BYTE(doc, offset + 1) == 'r') &&
            (DOC_BYTE(doc, offset + 2) == 'a') &&
            (DOC_BYTE(doc, offset + 3) == 'i') &&
            (DOC_BYTE(doc, offset + 4) == 'l') &&
            (DOC_BYTE(doc, offset + 5) == 'e') &&
            (DOC_BYTE(doc, offset + 6) == 'r')) {
            *offset_out = offset;
            return NSPDFERROR_OK;
        }
    }
    return NSPDFERROR_SYNTAX;
}


static nspdferror
decode_trailer(struct nspdf_doc *doc,
               uint64_t *offset_out,
               struct cos_object **trailer_out)
{
    struct cos_object *trailer;
    int res;
    uint64_t offset;

    offset = *offset_out;

    /* trailer object header */
    if ((DOC_BYTE(doc, offset    ) != 't') &&
        (DOC_BYTE(doc, offset + 1) != 'r') &&
        (DOC_BYTE(doc, offset + 2) != 'a') &&
        (DOC_BYTE(doc, offset + 3) != 'i') &&
        (DOC_BYTE(doc, offset + 4) != 'l') &&
        (DOC_BYTE(doc, offset + 5) != 'e') &&
        (DOC_BYTE(doc, offset + 6) != 'r')) {
        return -1;
    }
    offset += 7;
    doc_skip_ws(doc, &offset);

    res = cos_parse_object(doc, &offset, &trailer);
    if (res != 0) {
        return res;
    }

    if (trailer->type != COS_TYPE_DICTIONARY) {
        cos_free_object(trailer);
        return -1;
    }

    *trailer_out = trailer;
    *offset_out = offset;

    return NSPDFERROR_OK;
}


static nspdferror
decode_xref(struct nspdf_doc *doc, uint64_t *offset_out)
{
    uint64_t offset;
    nspdferror res;
    uint64_t objnumber; /* current object number */
    uint64_t objcount;

    offset = *offset_out;

    /* xref object header */
    if ((DOC_BYTE(doc, offset    ) != 'x') &&
        (DOC_BYTE(doc, offset + 1) != 'r') &&
        (DOC_BYTE(doc, offset + 2) != 'e') &&
        (DOC_BYTE(doc, offset + 3) != 'f')) {
        return NSPDFERROR_SYNTAX;
    }
    offset += 4;

    res = doc_skip_ws(doc, &offset);
    if (res != NSPDFERROR_OK) {
        return res;
    }

    /* subsections
     * <first object number> <number of references in subsection>
     */
    res = doc_read_uint(doc, &offset, &objnumber);
    while (res == NSPDFERROR_OK) {
        uint64_t lastobj;
        res = doc_skip_ws(doc, &offset);
        if (res != NSPDFERROR_OK) {
            return res;
        }

        res = doc_read_uint(doc, &offset, &objcount);
        if (res != NSPDFERROR_OK) {
            return res;
        }

        res = doc_skip_ws(doc, &offset);
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
            res = doc_read_uint(doc, &offset, &objindex);
            if (res != NSPDFERROR_OK) {
                return res;
            }
            offset++; /* skip space */

            res = doc_read_uint(doc, &offset, &objgeneration);
            if (res != NSPDFERROR_OK) {
                return res;
            }
            offset++; /* skip space */

            if ((DOC_BYTE(doc, offset++) == 'n')) {
                if (objnumber < doc->xref_size) {
                    struct xref_table_entry *indobj;
                    indobj = doc->xref_table + objnumber;

                    indobj->ref.id = objnumber;
                    indobj->ref.generation = objgeneration;
                    indobj->offset = objindex;

                    //printf("xref %lld %lld -> %lld\n", objnumber, objgeneration, objindex);
                } else {
                    printf("index out of bounds\n");
                }
            }

            offset += 2; /* skip EOL */
        }

        res = doc_read_uint(doc, &offset, &objnumber);
    }

    return NSPDFERROR_OK;
}


/**
 * recursively parse trailers and xref tables
 */
static nspdferror
decode_xref_trailer(struct nspdf_doc *doc, uint64_t xref_offset)
{
    nspdferror res;
    uint64_t offset; /* the current data offset */
    uint64_t startxref; /* the value of the startxref field */
    struct cos_object *trailer; /* the current trailer */
    int64_t prev;

    offset = xref_offset;

    res = find_trailer(doc, &offset);
    if (res != NSPDFERROR_OK) {
        printf("failed to find last trailer\n");
        return res;
    }

    res = decode_trailer(doc, &offset, &trailer);
    if (res != NSPDFERROR_OK) {
        printf("failed to decode trailer\n");
        return res;
    }

    res = decode_startxref(doc, &offset, &startxref);
    if (res != NSPDFERROR_OK) {
        printf("failed to decode startxref\n");
        goto decode_xref_trailer_failed;
    }

    if (startxref != xref_offset) {
        printf("startxref and Prev value disagree\n");
    }

    if (doc->xref_table == NULL) {
        /* extract Size from trailer and create xref table large enough */
        int64_t size;

        res = cos_get_dictionary_int(doc, trailer, "Size", &size);
        if (res != NSPDFERROR_OK) {
            printf("trailer has no integer Size value\n");
            goto decode_xref_trailer_failed;
        }

        res = cos_extract_dictionary_value(trailer, "Root", &doc->root);
        if (res != NSPDFERROR_OK) {
            printf("no Root!\n");
            goto decode_xref_trailer_failed;
        }

        doc->xref_table = calloc(size, sizeof(struct xref_table_entry));
        if (doc->xref_table == NULL) {
            res = NSPDFERROR_NOMEM;
            goto decode_xref_trailer_failed;
        }
        doc->xref_size = size;

        res = cos_extract_dictionary_value(trailer, "Encrypt", &doc->encrypt);
        if ((res != NSPDFERROR_OK) && (res != NSPDFERROR_NOTFOUND)) {
            goto decode_xref_trailer_failed;
        }

        res = cos_extract_dictionary_value(trailer, "Info", &doc->info);
        if ((res != NSPDFERROR_OK) && (res != NSPDFERROR_NOTFOUND)) {
            goto decode_xref_trailer_failed;
        }

        res = cos_extract_dictionary_value(trailer, "ID", &doc->id);
        if ((res != NSPDFERROR_OK) && (res != NSPDFERROR_NOTFOUND)) {
            goto decode_xref_trailer_failed;
        }

    }

    /* check for prev ID key in trailer and recurse call if present */
    res = cos_get_dictionary_int(doc, trailer, "Prev", &prev);
    if (res == NSPDFERROR_OK) {
        res = decode_xref_trailer(doc, prev);
        if (res != NSPDFERROR_OK) {
            goto decode_xref_trailer_failed;
        }
    }

    offset = xref_offset;
    /** @todo deal with XrefStm (number) in trailer */

    res = decode_xref(doc, &offset);
    if (res != NSPDFERROR_OK) {
        printf("failed to decode xref table\n");
        goto decode_xref_trailer_failed;
    }

decode_xref_trailer_failed:
    cos_free_object(trailer);

    return res;
}


/**
 * decode non-linear pdf trailer data
 *
 * PDF have a structure nominally defined as header, body, cross reference
 * table and trailer. The body, cross reference table and trailer sections may
 * be repeated in a scheme known as "incremental updates"
 *
 * The strategy used here is to locate the end of the last trailer block which
 * contains a startxref token followed by a byte offset into the file of the
 * beginning of the cross reference table followed by a literal '%%EOF'
 *
 * the initial offset is used to walk back down a chain of xref/trailers until
 * the trailer does not contain a Prev entry and decode xref tables forwards to
 * overwrite earlier object entries with later ones.
 *
 * It is necessary to search forwards from the xref table to find the trailer
 * block because instead of the Prev entry pointing to the previous trailer
 * (from which we could have extracted the startxref to find the associated
 * xref table) it points to the previous xref block which we have to skip to
 * find the subsequent trailer.
 *
 */
static nspdferror decode_trailers(struct nspdf_doc *doc)
{
    nspdferror res;
    uint64_t offset; /* the current data offset */
    uint64_t startxref; /* the value of the first startxref field */

    res = find_startxref(doc, &offset);
    if (res != NSPDFERROR_OK) {
        printf("failed to find startxref\n");
        return res;
    }

    res = decode_startxref(doc, &offset, &startxref);
    if (res != NSPDFERROR_OK) {
        printf("failed to decode startxref\n");
        return res;
    }

    /* recurse down the xref and trailers */
    return decode_xref_trailer(doc, startxref);
}


/**
 * recursively decodes a page tree
 */
static nspdferror
decode_page_tree(struct nspdf_doc *doc,
                 struct cos_object *page_tree_node,
                 unsigned int *page_index)
{
    nspdferror res;
    const char *type;

    // Type = Pages
    res = cos_get_dictionary_name(doc, page_tree_node, "Type", &type);
    if (res != NSPDFERROR_OK) {
        return res;
    }

    if (strcmp(type, "Pages") == 0) {
        struct cos_object *kids;
        unsigned int kids_size;
        unsigned int kids_index;

        if (doc->page_table == NULL) {
            /* allocate top level page table */
            int64_t count;

            res = cos_get_dictionary_int(doc, page_tree_node, "Count", &count);
            if (res != NSPDFERROR_OK) {
                return res;
            }

            doc->page_table = calloc(count, sizeof(struct page_table_entry));
            if (doc->page_table == NULL) {
                return NSPDFERROR_NOMEM;
            }
            doc->page_table_size = count;
        }

        res = cos_get_dictionary_array(doc, page_tree_node, "Kids", &kids);
        if (res != NSPDFERROR_OK) {
            return res;
        }

        res = cos_get_array_size(doc, kids, &kids_size);
        if (res != NSPDFERROR_OK) {
            return res;
        }

        for (kids_index = 0; kids_index < kids_size; kids_index++) {
            struct cos_object *kid;

            res = cos_get_array_dictionary(doc, kids, kids_index, &kid);
            if (res != NSPDFERROR_OK) {
                return res;
            }

            res = decode_page_tree(doc, kid, page_index);
            if (res != NSPDFERROR_OK) {
                return res;
            }
        }

    } else if (strcmp(type, "Page") == 0) {
        struct page_table_entry *page;

        page = doc->page_table + (*page_index);

        /* required heritable resources */
        res = cos_heritable_dictionary_dictionary(doc,
                                                  page_tree_node,
                                                  "Resources",
                                                  &(page->resources));
        if (res != NSPDFERROR_OK) {
            return res;
        }

        /* required heritable mediabox */
        res = cos_heritable_dictionary_array(doc,
                                             page_tree_node,
                                             "MediaBox",
                                             &(page->mediabox));
        if (res != NSPDFERROR_OK) {
            return res;
        }

        /* optional page contents */
        res = cos_get_dictionary_value(doc,
                                       page_tree_node,
                                       "Contents",
                                       &(page->contents));
        if ((res != NSPDFERROR_OK) &&
            (res != NSPDFERROR_NOTFOUND)) {
            return res;
        }

        /*
        printf("page index:%d page:%p resources:%p mediabox:%p contents:%p\n",
               *page_index,
               page,
               page->resources,
               page->mediabox,
               page->contents);
        */

        (*page_index)++;
        res = NSPDFERROR_OK;
    } else {
        res = NSPDFERROR_FORMAT;
    }
    return res;
}


static nspdferror decode_catalog(struct nspdf_doc *doc)
{
    nspdferror res;
    struct cos_object *catalog;
    const char *type;
    struct cos_object *pages;
    unsigned int page_index = 0;

    res = cos_get_dictionary(doc, doc->root, &catalog);
    if (res != NSPDFERROR_OK) {
        return res;
    }

    // Type = Catalog
    res = cos_get_dictionary_name(doc, catalog, "Type", &type);
    if (res != NSPDFERROR_OK) {
        return res;
    }
    if (strcmp(type, "Catalog") != 0) {
        return NSPDFERROR_FORMAT;
    }

    // Pages
    res = cos_get_dictionary_dictionary(doc, catalog, "Pages", &pages);
    if (res != NSPDFERROR_OK) {
        return res;
    }

    res = decode_page_tree(doc, pages, &page_index);
    if (res != NSPDFERROR_OK) {
        return res;
    }

    return res;
}

/* exported interface documented in nspdf/document.h */
nspdferror nspdf_document_create(struct nspdf_doc **doc_out)
{
    struct nspdf_doc *doc;
    doc = calloc(1, sizeof(struct nspdf_doc));
    if (doc == NULL) {
        return NSPDFERROR_NOMEM;
    }

    *doc_out = doc;

    return NSPDFERROR_OK;
}

/* exported interface documented in nspdf/document.h */
nspdferror nspdf_document_destroy(struct nspdf_doc *doc)
{
    free(doc);

    return NSPDFERROR_OK;
}


/**
 * find the PDF comment marker to identify the start of the document
 */
static nspdferror check_header(struct nspdf_doc *doc)
{
    uint64_t offset; /* offset of characters being considered for header */
    for (offset = 0; offset < 1024; offset++) {
        if ((DOC_BYTE(doc, offset) == '%') &&
            (DOC_BYTE(doc, offset + 1) == 'P') &&
            (DOC_BYTE(doc, offset + 2) == 'D') &&
            (DOC_BYTE(doc, offset + 3) == 'F') &&
            (DOC_BYTE(doc, offset + 4) == '-') &&
            (DOC_BYTE(doc, offset + 5) == '1') &&
            (DOC_BYTE(doc, offset + 6) == '.')) {
            doc->start += offset;
            doc->length -= offset;

            /* \todo read number for minor */
            return NSPDFERROR_OK;
        }
    }
    return NSPDFERROR_NOTFOUND;
}

/* exported interface documented in nspdf/document.h */
nspdferror
nspdf_document_parse(struct nspdf_doc *doc,
                     const uint8_t *buffer,
                     uint64_t buffer_length)
{
    nspdferror res;

    doc->start = buffer;
    doc->length = buffer_length;

    res = check_header(doc);
    if (res != 0) {
        printf("header check failed\n");
        return res;
    }

    res = decode_trailers(doc);
    if (res != NSPDFERROR_OK) {
        printf("failed to decode trailers (%d)\n", res);
        return res;
    }

    res = decode_catalog(doc);
    if (res != NSPDFERROR_OK) {
        printf("failed to decode catalog (%d)\n", res);
        return res;
    }

    return res;
}
