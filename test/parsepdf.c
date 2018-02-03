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

#include <libwapcaplet/libwapcaplet.h>

#include <nspdf/document.h>
#include <nspdf/meta.h>
#include <nspdf/page.h>

static nspdferror
read_whole_pdf(const char *fname, uint8_t **buffer, uint64_t *buffer_length)
{
    FILE *f;
    off_t len;
    uint8_t *buf;
    size_t rd;

    f = fopen(fname, "r");
    if (f == NULL) {
        perror("pdf open");
        return NSPDFERROR_NOTFOUND;
    }

    fseek(f, 0, SEEK_END);
    len = ftello(f);

    buf = malloc(len);
    fseek(f, 0, SEEK_SET);

    rd = fread(buf, len, 1, f);
    if (rd != 1) {
        perror("pdf read");
        free(buf);
        return 1;
    }

    fclose(f);

    *buffer = buf;
    *buffer_length = len;

    return NSPDFERROR_OK;
}

static nspdferror
pdf_path(const struct nspdf_style *style,
         const float *path,
         unsigned int path_length,
         const float transform[6],
         const void *ctxin)
{
        return NSPDFERROR_OK;
}

static nspdferror render_pages(struct nspdf_doc *doc, unsigned int page_count)
{
    nspdferror res;
    struct nspdf_render_ctx render_ctx;
    unsigned int page_render_list[4] = { 0, 1, 0, 1};
    unsigned int page_index;
    float page_width;
    float page_height;

    render_ctx.device_space[0] = 1;
    render_ctx.device_space[1] = 0;
    render_ctx.device_space[2] = 0;
    render_ctx.device_space[3] = -1; /* y scale */
    render_ctx.device_space[4] = 0; /* x offset */
    render_ctx.device_space[5] = 800; /* y offset */
    render_ctx.path = pdf_path;

    for (page_index = 0; page_index < page_count; page_index++) {
        res = nspdf_get_page_dimensions(doc,
                                        page_index,
                                        &page_width,
                                        &page_height);
        printf("page w:%f h:%f\n", page_width, page_height);

        res = nspdf_page_render(doc, page_index, &render_ctx);
        if (res != NSPDFERROR_OK) {
            break;
        }
    }

    for (page_index = 0; page_index < 4; page_index++) {
        res = nspdf_get_page_dimensions(doc,
                                        page_index,
                                        &page_width,
                                        &page_height);
        printf("page w:%f h:%f\n", page_width, page_height);

        res = nspdf_page_render(doc, page_render_list[page_index], &render_ctx);
        if (res != NSPDFERROR_OK) {
            break;
        }
    }

    return res;
}

int main(int argc, char **argv)
{
    uint8_t *buffer;
    uint64_t buffer_length;
    struct nspdf_doc *doc;
    nspdferror res;
    struct lwc_string_s *title;
    unsigned int page_count;

    if (argc < 2) {
        fprintf(stderr, "Usage %s <filename>\n", argv[0]);
        return 1;
    }

    res = read_whole_pdf(argv[1], &buffer, &buffer_length);
    if (res != 0) {
        printf("failed to read file\n");
        return res;
    }

    res = nspdf_document_create(&doc);
    if (res != NSPDFERROR_OK) {
        printf("failed to create a document\n");
        return res;
    }

    res = nspdf_document_parse(doc, buffer, buffer_length);
    if (res != NSPDFERROR_OK) {
        printf("document parse failed (%d)\n", res);
        return res;
    }

    res = nspdf_get_title(doc, &title);
    if (res == NSPDFERROR_OK) {
        printf("Title:%s\n", lwc_string_data(title));
    }

    res = nspdf_page_count(doc, &page_count);
    if (res == NSPDFERROR_OK) {
        printf("Pages:%d\n", page_count);
    }

    res = render_pages(doc, page_count);
        if (res != NSPDFERROR_OK) {
            printf("page render failed (%d)\n", res);
            return res;
        }

    res = nspdf_document_destroy(doc);
    if (res != NSPDFERROR_OK) {
        printf("failed to destroy document (%d)\n", res);
        return res;
    }

    free(buffer);

    return 0;
}
