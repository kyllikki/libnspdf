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
 * NetSurf PDF library page manipulation.
 */

#ifndef NSPDF_PAGE_H_
#define NSPDF_PAGE_H_

#include <nspdf/errors.h>

struct nspdf_doc;

/**
 * Type of plot operation
 */
enum nspdf_style_operation {
    NSPDF_OP_TYPE_NONE = 0, /**< No operation */
    NSPDF_OP_TYPE_SOLID, /**< Solid colour */
    NSPDF_OP_TYPE_DOT, /**< Dotted plot */
    NSPDF_OP_TYPE_DASH, /**< Dashed plot */
};


/**
 * Plot style for stroke/fill plotters
 */
typedef struct nspdf_style {
    enum nspdf_style_operation stroke_type; /**< Stroke plot type */
    float stroke_width; /**< Width of stroke, in pixels */
    uint32_t stroke_colour; /**< Colour of stroke XBGR */

    enum nspdf_style_operation fill_type; /**< Fill plot type */
    uint32_t fill_colour; /**< Colour of fill XBGR */
} nspdf_style;


enum nspdf_path_command {
    NSPDF_PATH_MOVE = 0,
    NSPDF_PATH_CLOSE,
    NSPDF_PATH_LINE,
    NSPDF_PATH_BEZIER,
};

struct nspdf_render_ctx {
    const void *ctx; /**< context passed to drawing functions */

    float device_space[6]; /* user space to device space transformation matrix */

    /**
     * Plots a path.
     *
     * Path plot consisting of lines and cubic Bezier curves. Line and fill
     *  colour is controlled by the style. All elements of the path are in
     *  device space.
     *
     * \param style Style controlling the path plot.
     * \param p elements of path
     * \param n nunber of elements on path
     * \param transform A transform to apply to the path.
     * \param ctx The drawing context.
     * \return NSERROR_OK on success else error code.
     */
    nspdferror (*path)(const struct nspdf_style *style, const float *p, unsigned int n, const float transform[6], const void *ctx);
};

nspdferror nspdf_get_page_dimensions(struct nspdf_doc *doc, unsigned int page_number, float *width, float *height);

nspdferror nspdf_page_count(struct nspdf_doc *doc, unsigned int *pages_out);

nspdferror nspdf_page_render(struct nspdf_doc *doc, unsigned int page_num, struct nspdf_render_ctx* render_ctx);

#endif /* NSPDF_META_H_ */
