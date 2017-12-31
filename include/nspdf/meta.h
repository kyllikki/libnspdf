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
 * NetSurf PDF library meta data about document.
 */

#ifndef NSPDF_META_H_
#define NSPDF_META_H_

#include <nspdf/errors.h>

struct nspdf_doc;
struct lwc_string_s;

nspdferror nspdf_get_title(struct nspdf_doc *doc, struct lwc_string_s **title);

#endif /* NSPDF_META_H_ */
