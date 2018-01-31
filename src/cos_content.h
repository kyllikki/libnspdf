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
 * NetSurf PDF library parsed content stream
 */

#ifndef NSPDF__COS_CONTENT_H_
#define NSPDF__COS_CONTENT_H_

/**
 * content operator
 *
 * parameters types are listed as:
 *  tag -
 *  prp - properties
 *  num - floating point value
 */
enum content_operator {
    /**
     * close, fill and stroke path with nonzero winding rule.
     * b()
     */
    CONTENT_OP_b,

    /**
     * fill and stroke path using nonzero winding rule.
     * B()
     */
    CONTENT_OP_B,

    /**
     * close, fill and stroke path with even/odd rule
     * b*()
     */
    CONTENT_OP_b_,

    /**
     * fill and stroke path with even/odd rule
     * B*()
     */
    CONTENT_OP_B_,

    /**
     * tag prp               BDC
     * begin marked content sequence with property list
     */
    CONTENT_OP_BDC,

    /**
     * BI()
     * begin inline image
     */
    CONTENT_OP_BI,

    /**
     * tag                   BMC
     * begin marked content sequence
     */
    CONTENT_OP_BMC,

    /**
     * begin text
     * BT()
     */
    CONTENT_OP_BT,

    /**
     * begin compatability
     * BX()
     */
    CONTENT_OP_BX,

    /**
     * append curved segment to path
     * c(num x1, num y1, num x2, num y2, num x3, num y3)
     */
    CONTENT_OP_c,
    CONTENT_OP_cm,  /* a b c d e f            cm - concatinate matrix to current trasnsform matrix */
    CONTENT_OP_CS,  /* name                   CS - set colour space for stroking operations */
    CONTENT_OP_cs,  /* name                   cs - set colourspace for non stroke operations */
    CONTENT_OP_d,   /* array phase             d - set line dash pattern */
    CONTENT_OP_d0,  /* wx wy                  d0 - set glyph width in type 3 font */
    CONTENT_OP_d1,  /* wx wy llx lly urx ury  d1 - set glyph width and bounding box in type 3 font */
    CONTENT_OP_Do,  /* name                   Do - invoke named xobject */
    CONTENT_OP_DP,  /* tag prp                DP - define marked content point with property list */
    CONTENT_OP_EI,  /*                        EI - end of inline image */
    CONTENT_OP_EMC, /*                       EMC - end marked content sequence */
    CONTENT_OP_ET,  /*                        ET - end text object */
    CONTENT_OP_EX,  /*                        EX - end compatability section */
    CONTENT_OP_f,   /*                         f - fill path using nonzero winding rule */
    CONTENT_OP_F,   /*                         F - fill path using nonzero winding rule */
    CONTENT_OP_f_,  /*                        f* - fill path with even/odd rule */

    /**
     * set gray level for stroking operations
     * G(num gray)
     */
    CONTENT_OP_G,

    /**
     * set gray level for nonstroking operations
     * g(num gray)
     */
    CONTENT_OP_g,
    CONTENT_OP_gs,  /* dictName               gs - set parameters from graphics state directory */

    /**
     * close subpath
     * h()
     */
    CONTENT_OP_h,

    /**
     * set flatness tolerance
     * i(num flatness)
     */
    CONTENT_OP_i,
    CONTENT_OP_ID,  /*                        ID - begin inline image data */

    /**
     * set line join style (0, 1 or 2)
     * j(int linejoin)
     */
    CONTENT_OP_j,
    CONTENT_OP_J,   /* linecap                 J - sel line cap style (int 0, 1 or 2) */
    CONTENT_OP_K,   /* c m y k                 K - set cmyk colour for stroking operations */
    CONTENT_OP_k,   /* c m y k                 k - set cmyk colour for nonstroking operations */
    CONTENT_OP_l,   /* x y                     l - append straight line segment to path */
    CONTENT_OP_m,   /* x y                     m -  begin new subpath */

    /**
     * set mitre limit
     * M(num mitrelimit)
     */
    CONTENT_OP_M,
    CONTENT_OP_MP,  /* tag                    MP - define marked content point */
    CONTENT_OP_n,   /*                         n - end path without filling or stroking*/
    CONTENT_OP_q,   /*                         q - save graphics state */
    CONTENT_OP_Q,   /*                         Q - restore graphics state */
    CONTENT_OP_re,  /* x y w h                re - append rectangle to path */
    CONTENT_OP_RG,  /* r g b                  RG - stroke colour in DeviceRGB colourspace */
    CONTENT_OP_rg,  /* r g b                  rg - nonstroke colour in DeviceRGB colourspace */
    CONTENT_OP_ri,  /* intent                 ri - set color rendering intent */
    CONTENT_OP_s,   /*                         s - close and stroke path */
    CONTENT_OP_S,   /*                         S - stroke path */
    CONTENT_OP_SC,  /* c1 c...                SC - set colour for stroking operation. 1 3 or 4 params */
    CONTENT_OP_sc,  /* c1 c...                sc - same as SC for nonstroking operations */
    CONTENT_OP_SCN, /* c1 c... name          SCN - same as SC but extra colour spaces. max 32 params */
    CONTENT_OP_scn, /* c1 c... name          scn - same as SCN for nonstroking operations */
    CONTENT_OP_sh,  /* name                   sh - paint area defined by shading pattern */
    CONTENT_OP_T_,  /*                        T* - move to start of next text line */
    CONTENT_OP_Tc,  /* charspace              Tc - set character spacing */
    CONTENT_OP_Td,  /* tx ty                  Td - move text position */
    CONTENT_OP_TD,  /* tx ty                  TD - move text position and set leading */
    CONTENT_OP_Tf,  /* font size              Tf - select text font and size */
    CONTENT_OP_Tj,  /* string                 Tj - show text */
    CONTENT_OP_TJ,  /* array                  TJ - show text strings allowing individual positioning */
    CONTENT_OP_TL,  /* leading                TL - set text leading for T* ' " operators */
    CONTENT_OP_Tm,  /* a b c d e f            Tm - set the text matrix */
    CONTENT_OP_Tr,  /* render                 Tr - set rendering mode (int) */
    CONTENT_OP_Ts,  /* rise                   Ts - set text rise */
    CONTENT_OP_Tw,  /* wordspace              Tw - set word spacing */
    CONTENT_OP_Tz,  /* scale                  Tz - set horizontal scaling */
    CONTENT_OP_v,   /* x2 y2 x3 y3             v - append curved segment path */

    /**
     * set line width
     * w(num linewidth)
     */
    CONTENT_OP_w,
    CONTENT_OP_W,   /*                         W - set clipping path using nonzero winding rule */
    CONTENT_OP_W_,  /*                        W* - set clipping path using odd even rule */
    CONTENT_OP_y,   /* x1 y1 x3 y3             y - append curved segment to path */
    CONTENT_OP__,   /* string                  ' - move to next line and show text */
    CONTENT_OP___,  /* aw ac string            " - set word and char spacing, move to next line and show text */
};

/* six numbers is adequate for almost all operations */
#define content_number_size (6)

/* compute how long the embedded string can be without inflating the
 * structure. size of the pointer is used instead of unsigned int as that is
 * what will control the structure padding.
 */
#define content_string_intrnl_lngth ((sizeof(float) * content_number_size) - sizeof(uint8_t *))


struct content_operation {
    enum content_operator operator;

    union {
        float number[content_number_size];

        char *name;

        int64_t i[3];

        struct {
            unsigned int length;
            union {
                char cdata[content_string_intrnl_lngth];
                uint8_t *pdata;
            } u;
        } string;

        struct {
            unsigned int length;
            struct cos_object **values;
        } array;

        struct {
            char *name;
            float number;
        } namenumber;

        struct {
            unsigned int length;
            struct cos_object **values;
            int64_t i;
        } arrayint;

    } u;
};

/**
 * Synthetic parsed content object.
 */
struct cos_content {
    unsigned int length; /**< number of content operations */
    unsigned int alloc; /**< number of allocated operations */
    struct content_operation *operations;
};


const char* nspdf__cos_content_operator_name(enum content_operator operator);


/**
 * convert an operator and operand list into an operation
 */
nspdferror nspdf__cos_content_convert(enum content_operator operator, struct cos_object **operands, unsigned int *operand_idx, struct content_operation *operation_out);


#endif
