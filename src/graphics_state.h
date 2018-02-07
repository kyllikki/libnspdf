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
 * NetSurf PDF library graphics state
 */

#ifndef NSPDF__GRAPHICS_STATE_H_
#define NSPDF__GRAPHICS_STATE_H_

/**
 * colourspaces
 * \todo extend this with full list from section 4.5.2
 */
enum graphics_state_colorspace {
    GSDeviceGray = 0, /* Default */
    GSDeviceRGB,
    GSDeviceCMYK,
};

struct graphics_state_color {
    enum graphics_state_colorspace space;
    union {
        float gray; /* default is 0 - black */
        struct {
            float r;
            float g;
            float b;
        } rgb;
        struct {
            float c;
            float m;
            float y;
            float k;
        } cmyk;
    } u;
};

struct graphics_state_param {
    float ctm[6]; /* current transform matrix */
    /* clipping path */
    struct {
        struct graphics_state_color colour;
    } stroke;
    struct {
        struct graphics_state_color colour;
    } other;
    /* text state */
    float line_width;
    unsigned int line_cap;
    unsigned int line_join;
    float miter_limit;
    /* dash pattern */
    /* rendering intent RelativeColorimetric */
    bool stroke_adjustment;
    /* blend mode: Normal */
    /* soft mask */
    /* alpha constant */
    /* alpha source */

    /* device dependant */
    bool overprint;
    float overprint_mode;
    /* black generation */
    /* undercolor removal */
    /* transfer */
    /* halftone */
    float flatness;
    float smoothness;
};

struct graphics_state {
    float *path; /* current path */
    unsigned int path_idx; /* current index into path */
    unsigned int path_alloc; /* current number of path elements allocated */

    struct graphics_state_param *param_stack; /* parameter stack */
    unsigned int param_stack_idx;
    unsigned int param_stack_alloc;
};

#endif
