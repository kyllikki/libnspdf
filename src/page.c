/*
 * Copyright 2018 Vincent Sanders <vince@netsurf-browser.org>
 *
 * This file is part of libnspdf.
 *
 * Licensed under the MIT License,
 *                http://www.opensource.org/licenses/mit-license.php
 */

#include <stdint.h>
#include <stdbool.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <nspdf/page.h>

#include "cos_object.h"
#include "pdf_doc.h"

/** page entry */
struct page_table_entry {
    struct cos_object *resources;
    struct cos_object *mediabox;
    struct cos_object *contents;
};

/**
 * recursively decodes a page tree
 */
nspdferror
nspdf__decode_page_tree(struct nspdf_doc *doc,
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

            res = nspdf__decode_page_tree(doc, kid, page_index);
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
        printf("page index:%d page:%p resources:%p mediabox:%p contents:%p contents type:%d\n",
               *page_index,
               page,
               page->resources,
               page->mediabox,
               page->contents,
            page->contents->type);
        */

        (*page_index)++;
        res = NSPDFERROR_OK;
    } else {
        res = NSPDFERROR_FORMAT;
    }
    return res;
}

/* exported interface documented in nspdf/page.h */
nspdferror
nspdf_page_count(struct nspdf_doc *doc, unsigned int *pages_out)
{
    *pages_out = doc->page_table_size;
    return NSPDFERROR_OK;
}

static nspdferror
nspdf__render_content_stream(struct nspdf_doc *doc,
                             struct page_table_entry *page_entry,
                             struct cos_object *content_entry)
{
    nspdferror res;
    struct cos_content *content_operations;

    res = cos_get_content(doc, content_entry, &content_operations);
    if (res == NSPDFERROR_OK) {
        printf("%p", content_operations);
    }

    return res;
}

/* exported interface documented in nspdf/page.h */
nspdferror
nspdf_page_render(struct nspdf_doc *doc, unsigned int page_number)
{
    struct page_table_entry *page_entry;
    struct cos_object *content_array;
    nspdferror res;

    page_entry = doc->page_table + page_number;

    /* contents may be an array of stream objects or just a single one */
    res = cos_get_array(doc, page_entry->contents, &content_array);
    if (res == NSPDFERROR_OK) {
        unsigned int content_stream_count;
        unsigned int content_stream_index;

        res = cos_get_array_size(doc, content_array, &content_stream_count);
        if (res != NSPDFERROR_OK) {
            return res;
        }
        for (content_stream_index = 0;
             content_stream_index < content_stream_count;
             content_stream_index++) {
            struct cos_object *content_entry;
            res = cos_get_array_value(doc,
                                content_array,
                                content_stream_index,
                                &content_entry);
            if (res != NSPDFERROR_OK) {
                break;
            }

            res = nspdf__render_content_stream(doc, page_entry, content_entry);
            if (res != NSPDFERROR_OK) {
                break;
            }
        }
    } else if (res == NSPDFERROR_TYPE) {
        res = nspdf__render_content_stream(doc, page_entry, page_entry->contents);
    }

    return res;
}
