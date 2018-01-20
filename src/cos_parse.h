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
 * NetSurf PDF library parsing cos objects from file
 */

#ifndef NSPDF__COS_PARSE_H_
#define NSPDF__COS_PARSE_H_

#include "cos_stream.h"

struct nspdf_doc;
struct cos_object;

/**
 * Parse input stream into an object
 *
 * lex and parse a byte stream to generate a COS object.
 */
nspdferror cos_parse_object(struct nspdf_doc *doc, struct cos_stream *stream, strmoff_t *offset_out, struct cos_object **cosobj_out);

/**
 * Parse content stream into content operations object
 */
nspdferror cos_parse_content_streams(struct nspdf_doc *doc, struct cos_stream **streams, unsigned int stream_count, struct cos_object **content_out);


#endif
