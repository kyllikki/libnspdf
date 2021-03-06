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
 * NetSurf PDF library byte classification
 */

#ifndef NSPDF__BYTE_CLASS_H_
#define NSPDF__BYTE_CLASS_H_

#define BC_RGLR 0 /* regular character */
#define BC_WSPC 1 /* character is whitespace */
#define BC_EOLM (1<<1) /* character signifies end of line */
#define BC_OCTL (1<<2) /* character is octal */
#define BC_DCML (1<<3) /* character is decimal */
#define BC_HEXL (1<<4) /* character is hexadecimal */
#define BC_DELM (1<<5) /* character is a delimiter */
#define BC_CMNT (1<<6) /* character is a comment */

const uint8_t *bclass;

#endif
