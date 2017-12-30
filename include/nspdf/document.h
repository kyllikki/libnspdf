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
 * NetSurf PDF library document handling
 */

#ifndef NSPDF_DOCUMENT_H_
#define NSPDF_DOCUMENT_H_

#include <nspdf/errors.h>

struct nspdf_doc;

/**
 * create a new PDF document
 */
nspdferror nspdf_document_create(struct nspdf_doc **doc_out);

/**
 * destroys a previously created document
 *
 * any allocated resources are freed but any buffers passed for parse are not
 * altered and may now be freed by the caller.
 */
nspdferror nspdf_document_destroy(struct nspdf_doc *doc);

/**
 * parse a PDF from a memory buffer
 *
 * reads all metadata and validates header, trailer, xref table and page tree
 * ready to render pages. The passed buffer ownership is transfered and must
 * not be altered untill the document is destroyed.
 */
nspdferror nspdf_document_parse(struct nspdf_doc *doc, const uint8_t *buffer, uint64_t buffer_length);


#endif /* NSPDF_DOCUMENT_H_ */
