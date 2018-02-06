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

#include "graphics_state.h"
#include "cos_content.h"
#include "cos_object.h"
#include "pdf_doc.h"

/** page entry */
struct page_table_entry {
    struct cos_object *resources;
    struct cos_object *contents;
    struct cos_rectangle mediabox; /* extent of media - required */
    struct cos_rectangle cropbox; /* default is mediabox */
    struct cos_rectangle bleedbox; /* default is crop box */
    struct cos_rectangle trimbox; /* default is crop box */
    struct cos_rectangle artbox; /* default is crop box */

};

/**
 * multiply pdf matricies
 *
 * pdf specifies its 3 x 3 transform matrix as six values and three constants
 *        | t[0] t[1]  0 |
 *   Mt = | t[2] t[3]  0 |
 *        | t[4] t[5]  1 |
 *
 * this multiples two such matricies together
 *   Mo = Ma * Mb
 *
 * Basic matrix expansion is
 *   | a b c |   | A B C |   | aA+bP+cU aB+bQ+cV aC+bR+cW |
 *   | p q r | * | P Q R | = | pA+qP+rU pB+qQ+rV pC+qR+rW |
 *   | u v w |   | U V W |   | uA+vP+wU uB+vQ+wV uC+vR+wW |
 *
 * With the a and b arrays substituted
 *   | o[0] o[1] 0 |
 *   | o[2] o[3] 0 | =
 *   | o[4] o[5] 1 |
 *
 *   | a[0] a[1] 0 |   | b[0] b[1] 0 |
 *   | a[2] a[3] 0 | * | b[2] b[3] 0 | =
 *   | a[4] a[5] 1 |   | b[4] b[5] 1 |
 *
 *   | a[0]*b[0]+a[1]*b[2]      a[0]*b[1]+a[1]*b[3]      0 |
 *   | a[2]*b[0]+a[3]*b[2]      a[2]*b[1]+a[3]*b[3]      0 |
 *   | a[4]*b[0]+a[5]*b[2]+b[4] a[4]*b[1]+a[5]*b[3]+b[5] 1 |
 *
 * \param a The array of six values for matrix a
 * \param b The array of six values for matrix b
 * \param o An array to receive six values resulting from Ma * Mb may be same array as a or b
 * \return NSPDFERROR_OK on success
 */
static nspdferror
pdf_matrix_multiply(float *a, float *b, float *o)
{
    float out[6]; /* result matrix array */

    out[0] = a[0]*b[0] + a[1]*b[2];
    out[1] = a[0]*b[1] + a[1]*b[3];
    out[2] = a[2]*b[0] + a[3]*b[2];
    out[3] = a[2]*b[1] + a[3]*b[3];
    out[4] = a[4]*b[0] + a[5]*b[2] + b[4];
    out[5] = a[4]*b[1] + a[5]*b[3] + b[5];

    /* calculate and then assign output to allow input and output arrays to
     * overlap
     */
    o[0] = out[0];
    o[1] = out[1];
    o[2] = out[2];
    o[3] = out[3];
    o[4] = out[4];
    o[5] = out[5];

    return NSPDFERROR_OK;
}

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
        struct cos_object *rect_array;

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
                                             &rect_array);
        if (res != NSPDFERROR_OK) {
            return res;
        }

        res = cos_get_rectangle(doc, rect_array, &page->mediabox);
        if (res != NSPDFERROR_OK) {
            return res;
        }

        /* optional heritable crop box */
        res = cos_heritable_dictionary_array(doc,
                                             page_tree_node,
                                             "CropBox",
                                             &rect_array);
        if (res == NSPDFERROR_OK) {
            res = cos_get_rectangle(doc, rect_array, &page->cropbox);
        }
        if (res != NSPDFERROR_OK) {
            /* default is mediabox */
            page->cropbox = page->mediabox;
        }

        /* optional bleed box */
        res = cos_get_dictionary_array(doc,
                                       page_tree_node,
                                       "BleedBox",
                                       &rect_array);
        if (res == NSPDFERROR_OK) {
            res = cos_get_rectangle(doc, rect_array, &page->bleedbox);
        }
        if (res != NSPDFERROR_OK) {
            /* default is cropbox */
            page->bleedbox = page->cropbox;
        }

        /* optional trim box */
        res = cos_get_dictionary_array(doc,
                                       page_tree_node,
                                       "TrimBox",
                                       &rect_array);
        if (res == NSPDFERROR_OK) {
            res = cos_get_rectangle(doc, rect_array, &page->trimbox);
        }
        if (res != NSPDFERROR_OK) {
            /* default is cropbox */
            page->trimbox = page->cropbox;
        }

        /* optional art box */
        res = cos_get_dictionary_array(doc,
                                       page_tree_node,
                                       "ArtBox",
                                       &rect_array);
        if (res == NSPDFERROR_OK) {
            res = cos_get_rectangle(doc, rect_array, &page->artbox);
        }
        if (res != NSPDFERROR_OK) {
            /* default is cropbox */
            page->artbox = page->cropbox;
        }

        /* optional page contents */
        res = cos_extract_dictionary_value(doc,
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


static inline nspdferror
render_operation_m(struct content_operation *operation, struct graphics_state *gs)
{
    gs->path[gs->path_idx++] = NSPDF_PATH_MOVE;
    gs->path[gs->path_idx++] = operation->u.number[0];
    gs->path[gs->path_idx++] = operation->u.number[1];
    return NSPDFERROR_OK;
}

static inline nspdferror
render_operation_l(struct content_operation *operation, struct graphics_state *gs)
{
    gs->path[gs->path_idx++] = NSPDF_PATH_LINE;
    gs->path[gs->path_idx++] = operation->u.number[0];
    gs->path[gs->path_idx++] = operation->u.number[1];
    return NSPDFERROR_OK;
}

static inline nspdferror
render_operation_c(struct content_operation *operation, struct graphics_state *gs)
{
    gs->path[gs->path_idx++] = NSPDF_PATH_BEZIER;
    gs->path[gs->path_idx++] = operation->u.number[0];
    gs->path[gs->path_idx++] = operation->u.number[1];
    gs->path[gs->path_idx++] = operation->u.number[2];
    gs->path[gs->path_idx++] = operation->u.number[3];
    gs->path[gs->path_idx++] = operation->u.number[4];
    gs->path[gs->path_idx++] = operation->u.number[5];
    return NSPDFERROR_OK;
}

static inline nspdferror
render_operation_re(struct content_operation *operation, struct graphics_state *gs)
{
    gs->path[gs->path_idx++] = NSPDF_PATH_MOVE;
    gs->path[gs->path_idx++] = operation->u.number[0]; /* x */
    gs->path[gs->path_idx++] = operation->u.number[1]; /* y */
    gs->path[gs->path_idx++] = NSPDF_PATH_LINE;
    gs->path[gs->path_idx++] = operation->u.number[0] + operation->u.number[2];
    gs->path[gs->path_idx++] = operation->u.number[1];
    gs->path[gs->path_idx++] = NSPDF_PATH_LINE;
    gs->path[gs->path_idx++] = operation->u.number[0] + operation->u.number[2];
    gs->path[gs->path_idx++] = operation->u.number[1] + operation->u.number[3];
    gs->path[gs->path_idx++] = NSPDF_PATH_LINE;
    gs->path[gs->path_idx++] = operation->u.number[0];
    gs->path[gs->path_idx++] = operation->u.number[1] + operation->u.number[3];
    gs->path[gs->path_idx++] = NSPDF_PATH_CLOSE;

    return NSPDFERROR_OK;
}

static inline nspdferror
render_operation_h(struct graphics_state *gs)
{
    gs->path[gs->path_idx++] = NSPDF_PATH_CLOSE;
    return NSPDFERROR_OK;
}

static inline nspdferror
render_operation_f(struct graphics_state *gs, struct nspdf_render_ctx* render_ctx)
{
    struct nspdf_style style;
    style.stroke_type = NSPDF_OP_TYPE_NONE;
    style.stroke_colour = 0x01000000;
    style.fill_type = NSPDF_OP_TYPE_SOLID;
    style.fill_colour = 0;

    render_ctx->path(&style,
                     gs->path,
                     gs->path_idx,
                     gs->param_stack[gs->param_stack_idx].ctm,
                     render_ctx->ctx);
    gs->path_idx = 0;
    return NSPDFERROR_OK;
}


static inline nspdferror
render_operation_S(struct graphics_state *gs, struct nspdf_render_ctx* render_ctx)
{
    struct nspdf_style style;

    style.stroke_type = NSPDF_OP_TYPE_SOLID;
    style.stroke_colour = 0;
    style.stroke_width = gs->param_stack[gs->param_stack_idx].line_width;
    style.fill_type = NSPDF_OP_TYPE_NONE;
    style.fill_colour = 0x01000000;
    render_ctx->path(&style,
                     gs->path,
                     gs->path_idx,
                     gs->param_stack[gs->param_stack_idx].ctm,
                     render_ctx->ctx);
    gs->path_idx = 0;
    return NSPDFERROR_OK;
}

static inline nspdferror
render_operation_w(struct content_operation *operation, struct graphics_state *gs)
{
    gs->param_stack[gs->param_stack_idx].line_width = operation->u.number[0];
    return NSPDFERROR_OK;
}

static inline nspdferror
render_operation_i(struct content_operation *operation, struct graphics_state *gs)
{
    gs->param_stack[gs->param_stack_idx].flatness = operation->u.number[0];
    return NSPDFERROR_OK;
}

static inline nspdferror
render_operation_M(struct content_operation *operation, struct graphics_state *gs)
{
    gs->param_stack[gs->param_stack_idx].miter_limit = operation->u.number[0];
    return NSPDFERROR_OK;
}

static inline nspdferror
render_operation_j(struct content_operation *operation, struct graphics_state *gs)
{
    gs->param_stack[gs->param_stack_idx].line_join = operation->u.i[0];
    return NSPDFERROR_OK;
}

static inline nspdferror
render_operation_J(struct content_operation *operation, struct graphics_state *gs)
{
    gs->param_stack[gs->param_stack_idx].line_cap = operation->u.i[0];
    return NSPDFERROR_OK;
}

static inline nspdferror
render_operation_q(struct graphics_state *gs)
{
    gs->param_stack[gs->param_stack_idx + 1] = gs->param_stack[gs->param_stack_idx];
    gs->param_stack_idx++;
    return NSPDFERROR_OK;
}

static inline nspdferror
render_operation_Q(struct graphics_state *gs)
{
    if (gs->param_stack_idx > 0) {
        gs->param_stack_idx--;
    }
    return NSPDFERROR_OK;
}



/**
 * pre-multiply matrix
 */
static inline nspdferror
render_operation_cm(struct content_operation *operation, struct graphics_state *gs)
{
    /* Mres = Mop * Mctm
     * where Mop is operation and Mctm is graphics state ctm
     */
    return pdf_matrix_multiply(operation->u.number,
                               gs->param_stack[gs->param_stack_idx].ctm,
                               gs->param_stack[gs->param_stack_idx].ctm);
}

/**
 * Initialise the parameter stack
 *
 * allocates the initial parameter stack and initialises the defaults
 */
static nspdferror
init_param_stack(struct graphics_state *gs, struct nspdf_render_ctx* render_ctx)
{
    gs->param_stack_alloc = 16; /* start with 16 deep parameter stack */
    gs->param_stack_idx = 0;
    gs->param_stack = calloc(gs->param_stack_alloc,
                             sizeof(struct graphics_state_param));
    if (gs->param_stack == NULL) {
        return NSPDFERROR_NOMEM;
    }

    gs->param_stack[0].ctm[0] = render_ctx->device_space[0];
    gs->param_stack[0].ctm[1] = render_ctx->device_space[1];
    gs->param_stack[0].ctm[2] = render_ctx->device_space[2];
    gs->param_stack[0].ctm[3] = render_ctx->device_space[3];
    gs->param_stack[0].ctm[4] = render_ctx->device_space[4];
    gs->param_stack[0].ctm[5] = render_ctx->device_space[5];
    gs->param_stack[0].line_width = 1.0;

    return NSPDFERROR_OK;
}

/* exported interface documented in nspdf/page.h */
nspdferror
nspdf_page_render(struct nspdf_doc *doc,
                  unsigned int page_number,
                  struct nspdf_render_ctx* render_ctx)
{
    struct page_table_entry *page_entry;
    struct cos_content *page_content; /* page operations array */
    nspdferror res;
    struct content_operation *operation;
    unsigned int idx;
    struct graphics_state gs;

    page_entry = doc->page_table + page_number;

    res = cos_get_content(doc, page_entry->contents, &page_content);
    if (res != NSPDFERROR_OK) {
        return res;
    }

    printf("page %d content:%p\n", page_number, page_content);

    gs.path_idx = 0;
    gs.path_alloc = 8192;
    gs.path = malloc(gs.path_alloc * sizeof(float));
    if (gs.path == NULL) {
        return NSPDFERROR_NOMEM;
    }

    res = init_param_stack(&gs, render_ctx);
    if (res != NSPDFERROR_OK) {
        free(gs.path);
        return res;
    }

    /* iterate over operations */
    for (idx = 0, operation = page_content->operations;
         idx < page_content->length;
         idx++, operation++) {
        switch(operation->operator) {
            /* path operations */
        case CONTENT_OP_m: /* move */
            res = render_operation_m(operation, &gs);
            break;

        case CONTENT_OP_l: /* line */
            res = render_operation_l(operation, &gs);
            break;

        case CONTENT_OP_re: /* rectangle */
            res = render_operation_re(operation, &gs);
            break;

        case CONTENT_OP_c: /* curve */
            res = render_operation_c(operation, &gs);
            break;

        case CONTENT_OP_h: /* close path */
            res = render_operation_h(&gs);
            break;

        case CONTENT_OP_f:
        case CONTENT_OP_f_:
        case CONTENT_OP_B:
        case CONTENT_OP_B_:
        case CONTENT_OP_b:
        case CONTENT_OP_b_:
            res = render_operation_f(&gs, render_ctx);
            break;

        case CONTENT_OP_s:
            render_operation_h(&gs);
            res = render_operation_S(&gs, render_ctx);
            break;

        case CONTENT_OP_S:
            res = render_operation_S(&gs, render_ctx);
            break;

            /* graphics state operations */
        case CONTENT_OP_w: /* line width */
            res = render_operation_w(operation, &gs);
            break;

        case CONTENT_OP_i: /* flatness */
            res = render_operation_i(operation, &gs);
            break;

        case CONTENT_OP_j: /* line join style */
            res = render_operation_j(operation, &gs);
            break;

        case CONTENT_OP_J: /* line cap style */
            res = render_operation_J(operation, &gs);
            break;

        case CONTENT_OP_M: /* miter limit */
            res = render_operation_M(operation, &gs);
            break;

        case CONTENT_OP_q: /* push parameter stack */
            res = render_operation_q(&gs);
            break;

        case CONTENT_OP_Q: /* pop parameter stack */
            res = render_operation_Q(&gs);
            break;

        case CONTENT_OP_cm: /* change matrix */
            res = render_operation_cm(operation, &gs);
            break;

        default:
            printf("operator %s\n",
               nspdf__cos_content_operator_name(operation->operator));
            break;

        }

    }

    free(gs.param_stack);
    free(gs.path);

    return res;
}


nspdferror
nspdf_get_page_dimensions(struct nspdf_doc *doc,
                          unsigned int page_number,
                          float *width,
                          float *height)
{
    struct page_table_entry *page_entry;
    page_entry = doc->page_table + page_number;
    *width = page_entry->cropbox.urx - page_entry->cropbox.llx;
    *height = page_entry->cropbox.ury - page_entry->cropbox.lly;
    return NSPDFERROR_OK;
}
