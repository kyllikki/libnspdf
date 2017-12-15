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
 *   whitespace - separate syntactic constructs like names and numbers treated
 *                as a single character except in comments, strings and streams
 *     end of line - characters that signify an end of line
 */
const uint8_t byte_classification[] = {
    BC_WSPC,           /* 00 - NULL */
    BC_RGLR,           /* 01 */
    BC_RGLR,           /* 02 */
    BC_RGLR,           /* 03 */
    BC_RGLR,           /* 04 */
    BC_RGLR,           /* 05 */
    BC_RGLR,           /* 06 */
    BC_RGLR,           /* 07 */
    BC_RGLR,           /* 08 */
    BC_WSPC,           /* 09 - HT */
    BC_WSPC | BC_EOLM, /* 0A - LF */
    BC_RGLR,           /* 0B */
    BC_WSPC,           /* 0C - FF */
    BC_WSPC | BC_EOLM, /* 0D - CR */
    BC_RGLR,           /* 0E */
    BC_RGLR,           /* 0F */
    BC_RGLR,
    BC_RGLR,
    BC_RGLR,
    BC_RGLR, /* 10 - 13 */
    BC_RGLR,
    BC_RGLR,
    BC_RGLR,
    BC_RGLR, /* 14 - 17 */
    BC_RGLR,
    BC_RGLR,
    BC_RGLR,
    BC_RGLR, /* 18 - 1B */
    BC_RGLR,
    BC_RGLR,
    BC_RGLR,
    BC_RGLR, /* 1C - 1F */
    BC_WSPC, /* 20 - SP */
    BC_RGLR,
    BC_RGLR,
    BC_RGLR, /* 20 - 23 */
    BC_RGLR,
    BC_DELM,                     /* '$' '%' */
    BC_RGLR,
    BC_RGLR,                     /* 26 - 27 */
    BC_DELM,
    BC_DELM,                     /* '(' ')' */
    BC_RGLR,
    BC_RGLR,                     /* 2A - 2B */
    BC_RGLR,
    BC_RGLR,                     /* 2C - 2D */
    BC_RGLR,
    BC_DELM,                     /* '.' '/' */
    BC_DCML | BC_HEXL,
    BC_DCML | BC_HEXL, /* '0' '1' */
    BC_DCML | BC_HEXL,
    BC_DCML | BC_HEXL, /* '2' '3' */
    BC_DCML | BC_HEXL,
    BC_DCML | BC_HEXL, /* '4' '5' */
    BC_DCML | BC_HEXL,
    BC_DCML | BC_HEXL, /* '6' '7' */
    BC_DCML | BC_HEXL,
    BC_DCML | BC_HEXL, /* '8' '9' */
    BC_RGLR,
    BC_RGLR,                     /* ':' ';' */
    BC_DELM,
    BC_RGLR,                     /* '<' '=' */
    BC_DELM,
    BC_RGLR,                     /* '>' '?' */
    BC_RGLR,
    BC_RGLR,
    BC_RGLR,
    BC_RGLR, /* 40 - 43 */
    BC_RGLR,
    BC_HEXL,
    BC_HEXL,
    BC_HEXL, /* 44 - 47 */
    BC_HEXL,
    BC_HEXL,
    BC_HEXL,
    BC_RGLR, /* 48 - 4B */
    BC_RGLR,
    BC_RGLR,
    BC_RGLR,
    BC_RGLR, /* 4C - 4F */
    BC_RGLR,
    BC_RGLR,
    BC_RGLR,
    BC_RGLR, /* 50 - 53 */
    BC_RGLR,
    BC_RGLR,
    BC_RGLR,
    BC_RGLR, /* 54 - 57 */
    BC_RGLR,
    BC_RGLR,                   /* 58 - 59 */
    BC_RGLR,
    BC_DELM,                   /* 'Z' '[' */
    BC_RGLR,
    BC_DELM,                   /* '\' ']' */
    BC_RGLR,
    BC_RGLR,                   /* 5E - 5F */
    BC_RGLR,
    BC_RGLR,
    BC_RGLR,
    BC_RGLR,
    BC_RGLR,
    BC_HEXL,
    BC_HEXL,
    BC_HEXL, /* 60 - 67 */
    BC_HEXL,
    BC_HEXL,
    BC_HEXL,
    BC_RGLR,
    BC_RGLR,
    BC_RGLR,
    BC_RGLR,
    BC_RGLR, /* 68 - 6F */
    BC_RGLR,
    BC_RGLR,
    BC_RGLR,
    BC_RGLR,
    BC_RGLR,
    BC_RGLR,
    BC_RGLR,
    BC_RGLR, /* 70 - 77 */
    BC_RGLR,
    BC_RGLR,                   /* 78 - 79 */
    BC_RGLR,
    BC_DELM,                   /* 'z' '{' */
    BC_RGLR,
    BC_DELM,                   /* '|' '}' */
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

const uint8_t *blcass = &byte_classification[0];
