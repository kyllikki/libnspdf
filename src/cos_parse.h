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

struct nspdf_doc;
struct cos_object;
struct cos_stream;

/**
 * Parse input stream into an object
 *
 * lex and parse a byte stream to generate a COS object.
 */
nspdferror cos_parse_object(struct nspdf_doc *doc, uint64_t *offset_out, struct cos_object **cosobj_out);

/**
 * Parse content stream into content operations object
 */
nspdferror cos_parse_content_stream(struct nspdf_doc *doc, struct cos_stream *stream, struct cos_object **content_out);

#endif
