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

#ifndef NSPDF__CONTENT_H_
#define NSPDF__CONTENT_H_

enum content_operator {
    CONTENT_OP_b,   /*   b - close, fill and stroke path with nonzero winding
                     *        rule. */
    CONTENT_OP_B,   /*   B - fill and stroke path using nonzero winding rule */
    CONTENT_OP_b_,  /*  b* - close, fill and stroke path with even/odd rule */
    CONTENT_OP_B_,  /*  B* - fill and stroke path with even/odd rule */
    CONTENT_OP_BDC, /* BDC - begin marked content sequence with property list */
    CONTENT_OP_BI,  /*  BI - begin inline image*/
    CONTENT_OP_BMC, /* BMC - begin marked content sequence */
    CONTENT_OP_BT,  /*  BT - begin text */
    CONTENT_OP_BX,  /*  BX - begin compatability */
    CONTENT_OP_c,   /*   c - append curved segment to path */
    CONTENT_OP_cm,  /*  cm - concatinate matrix to current trasnsform matrix */
    CONTENT_OP_CS,  /*  CS - set colour space for stroking operations */
    CONTENT_OP_cs,  /*  cs - set colourspace for non stroke operations */
    CONTENT_OP_d,   /*   d - set line dash pattern */
    CONTENT_OP_d0,  /*  d0 - set glyph width in type 3 font */
    CONTENT_OP_d1,  /*  d1 - set glyph width and bounding box in type 3 font */
    CONTENT_OP_Do,  /*  Do - invoke named xobject */
    CONTENT_OP_DP,  /*  DP - define marked content point with property list */
    CONTENT_OP_EI,  /*  EI - end of inline image */
    CONTENT_OP_EMC, /* EMC - end marked content sequence */
    CONTENT_OP_ET,  /*  ET - end text object */
    CONTENT_OP_EX,  /*  EX - end compatability section */
    CONTENT_OP_f,   /*   f - fill path using nonzero winding rule */
    CONTENT_OP_F,   /*   F - fill path using nonzero winding rule */
    CONTENT_OP_f_,  /*  f* - fill path with even/odd rule */
    CONTENT_OP_G,   /*   G - set gray level for stroking operations */
    CONTENT_OP_g,   /*   g - set gray level for nonstroking operations */
    CONTENT_OP_gs,  /*  gs - set parameters from graphics state directory */
    CONTENT_OP_h,   /*   h - close subpath */
    CONTENT_OP_i,   /*   i - set flatness tolerance */
    CONTENT_OP_ID,  /*  ID - begin inline image data */
    CONTENT_OP_j,   /*   j - set join style */
    CONTENT_OP_J,   /*   J - */
    CONTENT_OP_K,   /*   K - */
    CONTENT_OP_k,   /*   k - */
    CONTENT_OP_l,   /*   l - */
    CONTENT_OP_m,   /*   m - */
    CONTENT_OP_M,   /*   M - */
    CONTENT_OP_MP,  /*  MP - */
    CONTENT_OP_n,   /*   n - */
    CONTENT_OP_q,   /*   q - */
    CONTENT_OP_Q,   /*   Q - */
    CONTENT_OP_re,  /*  re - */
    CONTENT_OP_RG,  /*  RG - */
    CONTENT_OP_rg,  /*  rg - */
    CONTENT_OP_ri,  /*  ri - */
    CONTENT_OP_s,   /*   s - */
    CONTENT_OP_S,   /*   S - */
    CONTENT_OP_SC,  /*  SC - */
    CONTENT_OP_sc,  /*  sc - */
    CONTENT_OP_SCN, /* SCN - */
    CONTENT_OP_scn, /* scn - */
    CONTENT_OP_sh,  /*  sh - */
    CONTENT_OP_T_,  /*  T* - */
    CONTENT_OP_Tc,  /*  Tc - */
    CONTENT_OP_Td,  /*  Td - */
    CONTENT_OP_TD,  /*  TD - */
    CONTENT_OP_Tf,  /*  Tf - */
    CONTENT_OP_Tj,  /*  Tj - */
    CONTENT_OP_TJ,  /*  TJ - */
    CONTENT_OP_TL,  /*  TL - */
    CONTENT_OP_Tm,  /*  Tm - */
    CONTENT_OP_Tr,  /*  Tr - */
    CONTENT_OP_Ts,  /*  Ts - */
    CONTENT_OP_Tw,  /*  Tw - */
    CONTENT_OP_Tz,  /*  Tz - */
    CONTENT_OP_v,   /*   v - */
    CONTENT_OP_w,   /*   w - */
    CONTENT_OP_W,   /*   W - */
    CONTENT_OP_W_,  /*  W* - */
    CONTENT_OP_y,   /*   y - append curved segment to path */
    CONTENT_OP__,   /*   ' - move to next line and show text */
    CONTENT_OP___,  /*   " - set word and char spacing, move to next line and
                     *         show text */
};

struct content_operation
{
    enum content_operator operator;
    
};

#endif
