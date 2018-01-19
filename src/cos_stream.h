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
 * NetSurf PDF library COS stream
 */

#ifndef NSPDF__COS_STREAM_H_
#define NSPDF__COS_STREAM_H_

/* stream offset type */
typedef unsigned int strmoff_t;

/**
 * stream of data.
 */
struct cos_stream {
    strmoff_t length; /**< decoded stream length */
    size_t alloc; /**< memory allocated for stream */
    const uint8_t *data; /**< decoded stream data */
};

static inline uint8_t
stream_byte(struct cos_stream *stream, strmoff_t offset)
{
    return *(stream->data + offset);
}

#endif
