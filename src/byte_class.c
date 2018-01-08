/*
 * Copyright 2018 Vincent Sanders <vince@netsurf-browser.org>
 *
 * This file is part of libnspdf.
 *
 * Licensed under the MIT License,
 *                http://www.opensource.org/licenses/mit-license.php
 */

#include <stdint.h>

#include "byte_class.h"

/**
 * pdf byte classification
 *
 * spec defines three classes which this implementation futher subdivides for
 * comments, strings and streams:
 *   regular - the default class
 *     decimal - characters that appear in decimal values 0123456789
 *     hexidecimal - characters that appear in hex values 0123456789ABCDEF
 *   delimiter - The characters used to separate tokens ()[]{}<>/%
 *     comment - the % character used to introduce a comment
 *   whitespace - separate syntactic constructs like names and numbers treated
 *                as a single character except in comments, strings and streams
 *     end of line - characters that signify an end of line
 */
const uint8_t byte_classification[] = {
    BC_WSPC,                     /* 00 - NULL */
    BC_RGLR,                     /* 01 */
    BC_RGLR,                     /* 02 */
    BC_RGLR,                     /* 03 */
    BC_RGLR,                     /* 04 */
    BC_RGLR,                     /* 05 */
    BC_RGLR,                     /* 06 */
    BC_RGLR,                     /* 07 */
    BC_RGLR,                     /* 08 */
    BC_WSPC,                     /* 09 - HT */
    BC_WSPC | BC_EOLM,           /* 0A - LF */
    BC_RGLR,                     /* 0B */
    BC_WSPC,                     /* 0C - FF */
    BC_WSPC | BC_EOLM,           /* 0D - CR */
    BC_RGLR,                     /* 0E */
    BC_RGLR,                     /* 0F */
    BC_RGLR,                     /* 10 */
    BC_RGLR,                     /* 11 */
    BC_RGLR,                     /* 12 */
    BC_RGLR,                     /* 13 */
    BC_RGLR,                     /* 14 */
    BC_RGLR,                     /* 15 */
    BC_RGLR,                     /* 16 */
    BC_RGLR,                     /* 17 */
    BC_RGLR,                     /* 18 */
    BC_RGLR,                     /* 19 */
    BC_RGLR,                     /* 1A */
    BC_RGLR,                     /* 1B */
    BC_RGLR,                     /* 1C */
    BC_RGLR,                     /* 1D */
    BC_RGLR,                     /* 1E */
    BC_RGLR,                     /* 1F */
    BC_WSPC,                     /* 20 - SP */
    BC_RGLR,                     /* 21 */
    BC_RGLR,                     /* 22 */
    BC_RGLR,                     /* 23 */
    BC_RGLR,                     /* 24 - '$' */
    BC_DELM | BC_CMNT,           /* 25 - '%' */
    BC_RGLR,                     /* 26 */
    BC_RGLR,                     /* 27 */
    BC_DELM,                     /* 28 - '(' */
    BC_DELM,                     /* 29 - ')' */
    BC_RGLR,                     /* 2A */
    BC_RGLR,                     /* 2B */
    BC_RGLR,                     /* 2C */
    BC_RGLR,                     /* 2D */
    BC_RGLR,                     /* 2E - '.' */
    BC_DELM,                     /* 2F - '/' */
    BC_OCTL | BC_DCML | BC_HEXL, /* 30 - '0' */
    BC_OCTL | BC_DCML | BC_HEXL, /* 31 - '1' */
    BC_OCTL | BC_DCML | BC_HEXL, /* 32 - '2' */
    BC_OCTL | BC_DCML | BC_HEXL, /* 33 - '3' */
    BC_OCTL | BC_DCML | BC_HEXL, /* 34 - '4' */
    BC_OCTL | BC_DCML | BC_HEXL, /* 35 - '5' */
    BC_OCTL | BC_DCML | BC_HEXL, /* 36 - '6' */
    BC_OCTL | BC_DCML | BC_HEXL, /* 37 - '7' */
    BC_DCML | BC_HEXL,           /* 38 - '8' */
    BC_DCML | BC_HEXL,           /* 39 - '9' */
    BC_RGLR,                     /* 3A - ':' */
    BC_RGLR,                     /* 3B - ';' */
    BC_DELM,                     /* 3C - '<' */
    BC_RGLR,                     /* 3D - '=' */
    BC_DELM,                     /* 3E - '>' */
    BC_RGLR,                     /* 3F - '?' */
    BC_RGLR,                     /* 40 - */
    BC_HEXL,                     /* 41 - A */
    BC_HEXL,                     /* 42 - B */
    BC_HEXL,                     /* 43 - C */
    BC_HEXL,                     /* 44 - D */
    BC_HEXL,                     /* 45 - E */
    BC_HEXL,                     /* 46 - F */
    BC_RGLR,                     /* 47 - G */
    BC_RGLR,                     /* 48 - H */
    BC_RGLR,                     /* 49 - I */
    BC_RGLR,                     /* 4A - J */
    BC_RGLR,                     /* 4B - K */
    BC_RGLR,                     /* 4C - L */
    BC_RGLR,                     /* 4D - M */
    BC_RGLR,                     /* 4E - N */
    BC_RGLR,                     /* 4F - O */
    BC_RGLR,                     /* 50 - P */
    BC_RGLR,                     /* 51 - Q */
    BC_RGLR,                     /* 52 - R */
    BC_RGLR,                     /* 53 - S */
    BC_RGLR,                     /* 54 - T */
    BC_RGLR,                     /* 55 - U */
    BC_RGLR,                     /* 56 - V */
    BC_RGLR,                     /* 57 - W */
    BC_RGLR,                     /* 58 - X */
    BC_RGLR,                     /* 59 - Y */
    BC_RGLR,                     /* 5A - 'Z' */
    BC_DELM,                     /* 5B - '[' */
    BC_RGLR,                     /* 5C - '\' */
    BC_DELM,                     /* 5D - ']' */
    BC_RGLR,                     /* 5E */
    BC_RGLR,                     /* 5F */
    BC_RGLR,                     /* 60 */
    BC_HEXL,                     /* 61 - a */
    BC_HEXL,                     /* 62 - b */
    BC_HEXL,                     /* 63 - c */
    BC_HEXL,                     /* 64 - d */
    BC_HEXL,                     /* 65 - e */
    BC_HEXL,                     /* 66 - f */
    BC_RGLR,                     /* 67 - g */
    BC_RGLR,                     /* 68 - h */
    BC_RGLR,                     /* 69 - i */
    BC_RGLR,                     /* 6A - j */
    BC_RGLR,                     /* 6B - k */
    BC_RGLR,                     /* 6C - l */
    BC_RGLR,                     /* 6D - m */
    BC_RGLR,                     /* 6E - n */
    BC_RGLR,                     /* 6F - o */
    BC_RGLR,                     /* 70 - p */
    BC_RGLR,                     /* 71 - q */
    BC_RGLR,                     /* 72 - r */
    BC_RGLR,                     /* 73 - s */
    BC_RGLR,                     /* 74 - t */
    BC_RGLR,                     /* 75 - u */
    BC_RGLR,                     /* 76 - v */
    BC_RGLR,                     /* 77 - w */
    BC_RGLR,                     /* 78 - x */
    BC_RGLR,                     /* 79 - y */
    BC_RGLR,                     /* 7A - 'z' */
    BC_DELM,                     /* 7B - '{' */
    BC_RGLR,                     /* 7C - '|' */
    BC_DELM,                     /* 7D - '}' */
    BC_RGLR,
    BC_RGLR,                   /* 7E - 7F */
    BC_RGLR,
    BC_RGLR,
    BC_RGLR,
    BC_RGLR, /* 80 - 83 */
    BC_RGLR, BC_RGLR, BC_RGLR, BC_RGLR, /* 84 - 87 */
    BC_RGLR, BC_RGLR, BC_RGLR, BC_RGLR,
    BC_RGLR, BC_RGLR, BC_RGLR, BC_RGLR, /* 88 - 8F */
    BC_RGLR, BC_RGLR, BC_RGLR, BC_RGLR,
    BC_RGLR, BC_RGLR, BC_RGLR, BC_RGLR, /* 90 - 97 */
    BC_RGLR, BC_RGLR, BC_RGLR, BC_RGLR,
    BC_RGLR, BC_RGLR, BC_RGLR, BC_RGLR, /* 98 - 9F */
    BC_RGLR, BC_RGLR, BC_RGLR, BC_RGLR,
    BC_RGLR, BC_RGLR, BC_RGLR, BC_RGLR, /* A0 - A7 */
    BC_RGLR, BC_RGLR, BC_RGLR, BC_RGLR,
    BC_RGLR, BC_RGLR, BC_RGLR, BC_RGLR, /* A8 - AF */
    BC_RGLR, BC_RGLR, BC_RGLR, BC_RGLR,
    BC_RGLR, BC_RGLR, BC_RGLR, BC_RGLR, /* B0 - B7 */
    BC_RGLR, BC_RGLR, BC_RGLR, BC_RGLR,
    BC_RGLR, BC_RGLR, BC_RGLR, BC_RGLR, /* B8 - BF */
    BC_RGLR, BC_RGLR, BC_RGLR, BC_RGLR,
    BC_RGLR, BC_RGLR, BC_RGLR, BC_RGLR, /* C0 - C7 */
    BC_RGLR, BC_RGLR, BC_RGLR, BC_RGLR,
    BC_RGLR, BC_RGLR, BC_RGLR, BC_RGLR, /* C8 - CF */
    BC_RGLR, BC_RGLR, BC_RGLR, BC_RGLR,
    BC_RGLR, BC_RGLR, BC_RGLR, BC_RGLR, /* D0 - D7 */
    BC_RGLR, BC_RGLR, BC_RGLR, BC_RGLR,
    BC_RGLR, BC_RGLR, BC_RGLR, BC_RGLR, /* D8 - DF */
    BC_RGLR, BC_RGLR, BC_RGLR, BC_RGLR,
    BC_RGLR, BC_RGLR, BC_RGLR, BC_RGLR, /* E0 - E7 */
    BC_RGLR, BC_RGLR, BC_RGLR, BC_RGLR,
    BC_RGLR, BC_RGLR, BC_RGLR, BC_RGLR, /* E8 - EF */
    BC_RGLR, BC_RGLR, BC_RGLR, BC_RGLR,
    BC_RGLR, BC_RGLR, BC_RGLR, BC_RGLR, /* F0 - F7 */
    BC_RGLR, BC_RGLR, BC_RGLR, BC_RGLR,
    BC_RGLR, BC_RGLR, BC_RGLR, BC_RGLR, /* F8 - FF */
};

const uint8_t *bclass = &byte_classification[0];
