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
 * NetSurf PDF library return codes
 */

#ifndef NSPDF_ERRORS_H_
#define NSPDF_ERRORS_H_

typedef enum {
    NSPDFERROR_OK, /**< no error */
    NSPDFERROR_NOMEM, /**< memory allocation error */
    NSPDFERROR_SYNTAX, /**< syntax error in parse */
    NSPDFERROR_SIZE, /**< not enough input data */
    NSPDFERROR_RANGE, /**< value outside type range */
    NSPDFERROR_TYPE, /**< wrong type error */
    NSPDFERROR_NOTFOUND, /**< key not found */
    NSPDFERROR_FORMAT, /**< objects do not cornform to expected format */
    NSPDFERROR_INCOMPLETE, /**< operation was not completed */
    NSPDFERROR_REFERENCE, /**< unable to dereference object. */
} nspdferror;

#endif
