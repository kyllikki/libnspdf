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

nspdferror nspdf_page_count(struct nspdf_doc *doc, unsigned int *pages_out);

nspdferror nspdf_page_render(struct nspdf_doc *doc, unsigned int page_num);

#endif /* NSPDF_META_H_ */
