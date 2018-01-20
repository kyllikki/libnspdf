/*
 * Copyright 2018 Vincent Sanders <vince@netsurf-browser.org>
 *
 * This file is part of libnspdf.
 *
 * Licensed under the MIT License,
 *                http://www.opensource.org/licenses/mit-license.php
 */

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <nspdf/errors.h>

#include "cos_parse.h"
#include "byte_class.h"
#include "cos_object.h"
#include "content.h"
#include "pdf_doc.h"

/** increments in which cos string allocations are extended */
#define COS_STRING_ALLOC 32

/** Maximum length of cos name */
#define NAME_MAX_LENGTH 127


static nspdferror
cos_string_append(struct cos_string *s, uint8_t c)
{
    //printf("appending 0x%x to %p len %d alloc %d\n", c, s->data, s->length, s->alloc);
    if (s->length == s->alloc) {
        uint8_t *ns;
        ns = realloc(s->data, s->alloc + COS_STRING_ALLOC);
        if (ns == NULL) {
            return NSPDFERROR_NOMEM;
        }
        s->data = ns;
        s->alloc += COS_STRING_ALLOC;
    }
    s->data[s->length++] = c;
    return NSPDFERROR_OK;
}

static uint8_t xtoi(uint8_t x)
{
    if (x >= '0' && x <= '9') {
        x = x - '0';
    } else if (x >= 'a' && x <='f') {
        x = x - 'a' + 10;
    } else if (x >= 'A' && x <='F') {
        x = x - 'A' + 10;
    }
    return x;
}

/**
 * parse a number
 */
static nspdferror
cos_parse_number(struct cos_stream *stream,
                 strmoff_t *offset_out,
                 struct cos_object **cosobj_out)
{
    nspdferror res;
    struct cos_object *cosobj;
    uint8_t c; /* current byte from source data */
    unsigned int len; /* number of decimal places in number */
    uint8_t num[21]; /* temporary buffer for decimal values */
    strmoff_t offset; /* current offset of source data */
    unsigned int point;
    bool real = false;
    bool neg = false;

    offset = *offset_out;

    c = stream_byte(stream, offset);
    if (c == '-') {
        neg = true;
        offset++;
    } else if (c == '+') {
        neg = false;
        offset++;
    }

    for (len = 0; len < sizeof(num); len++) {
        c = stream_byte(stream, offset);

        if (c == '.') {
            real = true;
            point = len;
            offset++;
            c = stream_byte(stream, offset);
        }

        if ((bclass[c] & BC_DCML) != BC_DCML) {
            int64_t result = 0; /* parsed result */
            uint64_t tens;

            if (len == 0) {
                 /* parse error no decimals in input */
                return NSPDFERROR_SYNTAX;
            }

            point = len - point;

            /* sum value from each place */
            for (tens = 1; len > 0; tens = tens * 10, len--) {
                result += (num[len - 1] * tens);
            }

            res = nspdf__stream_skip_ws(stream, &offset);
            if (res != NSPDFERROR_OK) {
                return res;
            }

            cosobj = calloc(1, sizeof(struct cos_object));
            if (cosobj == NULL) {
                return NSPDFERROR_NOMEM;
            }

            if (real) {
                unsigned int div = 1;
                for (; point > 0;point--) {
                    div = div * 10;
                }
                cosobj->type = COS_TYPE_REAL;
                if (neg) {
                    cosobj->u.real = -((float)result / div);
                } else {
                    cosobj->u.real = (float)result / div;
                }
                //printf("real %d %f\n", result, cosobj->u.real);
            } else {
                cosobj->type = COS_TYPE_INT;
                if (neg) {
                    cosobj->u.i = -result;
                } else {
                    cosobj->u.i = result;
                }
            }

            *cosobj_out = cosobj;

            *offset_out = offset;

            return NSPDFERROR_OK;
        }
        num[len] = c - '0';
        offset++;
    }
    return NSPDFERROR_RANGE; /* number too long */
}


/**
 * parse literal string
 *
 */
static nspdferror
cos_parse_string(struct cos_stream *stream,
                  strmoff_t *offset_out,
                  struct cos_object **cosobj_out)
{
    strmoff_t offset;
    struct cos_object *cosobj;
    uint8_t c;
    unsigned int pdepth = 1; /* depth of open parens */
    struct cos_string *cstring;

    offset = *offset_out;

    c = stream_byte(stream, offset++);
    if (c != '(') {
        return NSPDFERROR_SYNTAX;
    }

    cstring = calloc(1, sizeof(*cstring));
    if (cstring == NULL) {
        return NSPDFERROR_NOMEM;
    }

    cosobj = calloc(1, sizeof(*cosobj));
    if (cosobj == NULL) {
        return NSPDFERROR_NOMEM;
    }
    cosobj->type = COS_TYPE_STRING;
    cosobj->u.s = cstring;

    while (pdepth > 0) {
        c = stream_byte(stream, offset++);

        if (c == ')') {
            pdepth--;
            if (pdepth == 0) {
                break;
            }
        } else if (c == '(') {
            pdepth++;
        } else if ((bclass[c] & BC_EOLM ) != 0) {
            /* unescaped end of line characters are translated to a single
             * newline
             */
            c = stream_byte(stream, offset);
            while ((bclass[c] & BC_EOLM) != 0) {
                offset++;
                c = stream_byte(stream, offset);
            }
            c = '\n';
        } else if (c == '\\') {
            /* escaped chars */
            c = stream_byte(stream, offset++);
            switch (c) {
            case 'n':
                c = '\n';
                break;

            case 'r':
                c = '\r';
                break;

            case 't':
                c = '\t';
                break;

            case 'b':
                c = '\b';
                break;

            case 'f':
                c = '\f';
                break;

            case '(':
                c = '(';
                break;

            case ')':
                c = ')';
                break;

            case '\\':
                c = '\\';
                break;

            default:

                if ((bclass[c] & BC_EOLM) != 0) {
                    /* escaped end of line, swallow it */
                    c = stream_byte(stream, offset++);
                    while ((bclass[c] & BC_EOLM) != 0) {
                        c = stream_byte(stream, offset++);
                    }
                } else if ((bclass[c] & BC_OCTL) != 0) {
                    /* octal value */
                    uint8_t val;
                    val = (c - '0');
                    c = stream_byte(stream, offset);
                    if ((bclass[c] & BC_OCTL) != 0) {
                        offset++;
                        val = (val << 3) | (c - '0');
                        c = stream_byte(stream, offset);
                        if ((bclass[c] & BC_OCTL) != 0) {
                            offset++;
                            val = (val << 3) | (c - '0');
                            c = val;
                        }
                    }
                } /* else invalid (skip backslash) */
                break;
            }
        }

        /* c contains the character to add to the string */
        cos_string_append(cstring, c);
    }

    nspdf__stream_skip_ws(stream, &offset);

    *cosobj_out = cosobj;
    *offset_out = offset;

    return NSPDFERROR_OK;
}

/**
 * decode hex encoded string
 */
static nspdferror
cos_parse_hex_string(struct cos_stream *stream,
                     strmoff_t *offset_out,
                     struct cos_object **cosobj_out)
{
    strmoff_t offset;
    struct cos_object *cosobj;
    uint8_t c;
    uint8_t value = 0;
    struct cos_string *cstring;
    bool first = true;

    offset = *offset_out;

    c = stream_byte(stream, offset++);
    if (c != '<') {
        return NSPDFERROR_SYNTAX;
    }

    cstring = calloc(1, sizeof(*cstring));
    if (cstring == NULL) {
        return NSPDFERROR_NOMEM;
    }

    cosobj = calloc(1, sizeof(*cosobj));
    if (cosobj == NULL) {
        return NSPDFERROR_NOMEM;
    }
    cosobj->type = COS_TYPE_STRING;
    cosobj->u.s = cstring;

    for (; offset < stream->length; offset++) {
        c = stream_byte(stream, offset);
        if (c == '>') {
            if (first == false) {
                cos_string_append(cstring, value);
            }
            offset++;
            nspdf__stream_skip_ws(stream, &offset);

            *cosobj_out = cosobj;
            *offset_out = offset;

            return NSPDFERROR_OK;
        } else if ((bclass[c] & BC_HEXL) != 0) {
            if (first) {
                value = xtoi(c) << 4;
                first = false;
            } else {
                value |= xtoi(c);
                first = true;
                cos_string_append(cstring, value);
            }
        } else if ((bclass[c] & BC_WSPC) == 0) {
            break; /* unknown byte value in string */
        }
    }
    return NSPDFERROR_SYNTAX;
}

/**
 * parse a COS dictionary
 */
static nspdferror
cos_parse_dictionary(struct nspdf_doc *doc,
                     struct cos_stream *stream,
                     strmoff_t *offset_out,
                     struct cos_object **cosobj_out)
{
    nspdferror res;
    strmoff_t offset;
    struct cos_object *cosobj;
    struct cos_dictionary_entry *entry;
    struct cos_object *key;
    struct cos_object *value;

    offset = *offset_out;

    if ((stream_byte(stream, offset    ) != '<') ||
        (stream_byte(stream, offset + 1) != '<')) {
        return NSPDFERROR_SYNTAX; /* syntax error */
    }
    offset += 2;
    nspdf__stream_skip_ws(stream, &offset);

    //printf("found a dictionary\n");

    cosobj = calloc(1, sizeof(struct cos_object));
    if (cosobj == NULL) {
        return NSPDFERROR_NOMEM;
    }
    cosobj->type = COS_TYPE_DICTIONARY;

    while ((stream_byte(stream, offset    ) != '>') &&
           (stream_byte(stream, offset + 1) != '>')) {

        res = cos_parse_object(doc, stream, &offset, &key);
        if (res != NSPDFERROR_OK) {
            printf("key object decode failed\n");
            goto cos_parse_dictionary_error;
        }
        if (key->type != COS_TYPE_NAME) {
            /* key value pairs without a name */
            printf("key was %d not a name %d\n", key->type, COS_TYPE_NAME);

            cos_free_object(key);
            res = NSPDFERROR_SYNTAX;
            goto cos_parse_dictionary_error;
        }

        res = cos_parse_object(doc, stream, &offset, &value);
        if (res != NSPDFERROR_OK) {
            printf("Unable to decode value object in dictionary\n");
            cos_free_object(key);
            goto cos_parse_dictionary_error;
        }

        /* add dictionary entry */
        entry = calloc(1, sizeof(struct cos_dictionary_entry));
        if (entry == NULL) {
            cos_free_object(key);
            cos_free_object(value);
            res = NSPDFERROR_NOMEM;
            goto cos_parse_dictionary_error;
        }
        //printf("key:%s value(type):%d\n", key->u.n, value->type);

        entry->key = key;
        entry->value = value;
        entry->next = cosobj->u.dictionary;

        cosobj->u.dictionary = entry;

    }
    offset += 2; /* skip closing >> */
    nspdf__stream_skip_ws(stream, &offset);

    *cosobj_out = cosobj;
    *offset_out = offset;

    return NSPDFERROR_OK;

cos_parse_dictionary_error:
    cos_free_object(cosobj);

    return res;
}


/**
 * parse a COS list
 */
static nspdferror
cos_parse_list(struct nspdf_doc *doc,
               struct cos_stream *stream,
               strmoff_t *offset_out,
               struct cos_object **cosobj_out)
{
    strmoff_t offset;
    struct cos_object *cosobj;
    struct cos_array *array;
    struct cos_object *value;
    nspdferror res;

    offset = *offset_out;

    /* sanity check first token is list open */
    if (stream_byte(stream, offset++) != '[') {
        printf("list does not start with a [\n");
        return NSPDFERROR_SYNTAX;
    }

    /* advance offset to next token */
    res = nspdf__stream_skip_ws(stream, &offset);
    if (res != NSPDFERROR_OK) {
        return res;
    }

    //printf("found a list\n");
    /* setup array object */
    cosobj = calloc(1, sizeof(struct cos_object));
    if (cosobj == NULL) {
        return NSPDFERROR_NOMEM;
    }
    cosobj->type = COS_TYPE_ARRAY;

    array = calloc(1, sizeof(struct cos_array));
    if (array == NULL) {
        cos_free_object(cosobj);
        return NSPDFERROR_NOMEM;
    }
    cosobj->u.array = array;

    while (stream_byte(stream, offset) != ']') {

        res = cos_parse_object(doc, stream, &offset, &value);
        if (res != NSPDFERROR_OK) {
            cos_free_object(cosobj);
            printf("Unable to decode value object in list\n");
            return res;
        }

        /* ensure there is enough space allocated for object pointers */
        if (array->alloc < (array->length + 1)) {
            struct cos_object **nvalues;
            nvalues = realloc(array->values,
                            sizeof(struct cos_object *) * (array->alloc + 32));
            if (nvalues == NULL) {
                cos_free_object(cosobj);
                return NSPDFERROR_NOMEM;
            }
            array->values = nvalues;
            array->alloc += 32;
        }

        *(array->values + array->length) = value;
        array->length++;
    }
    offset++; /* skip closing ] */

    nspdf__stream_skip_ws(stream, &offset);

    *cosobj_out = cosobj;
    *offset_out = offset;

    return NSPDFERROR_OK;
}


/**
 * decode a name object
 *
 * \todo deal with # symbols on pdf versions 1.2 and later
 */
static nspdferror
cos_parse_name(struct cos_stream *stream,
               strmoff_t *offset_out,
               struct cos_object **cosobj_out)
{
    strmoff_t offset;
    struct cos_object *cosobj;
    uint8_t c;
    char name[NAME_MAX_LENGTH + 1];
    int idx = 0;

    offset = *offset_out;

    c = stream_byte(stream, offset++);
    if (c != '/') {
        return NSPDFERROR_SYNTAX;/* names must be prefixed with a / */
    }
    //printf("found a name\n");

    c = stream_byte(stream, offset);
    while ((idx <= NAME_MAX_LENGTH) &&
           ((bclass[c] & (BC_WSPC | BC_DELM)) == 0)) {
        offset++;
        //printf("%c", c);
        name[idx++] = c;
        c = stream_byte(stream, offset);
    }

    //printf("\nidx: %d\n", idx);
    if (idx > NAME_MAX_LENGTH) {
        /* name length exceeded implementation limit */
        return NSPDFERROR_RANGE;
    }
    name[idx] = 0;

    //printf("name: %s\n", name);

    nspdf__stream_skip_ws(stream, &offset);

    cosobj = calloc(1, sizeof(struct cos_object));
    if (cosobj == NULL) {
        return NSPDFERROR_NOMEM; /* memory error */
    }

    cosobj->type = COS_TYPE_NAME;
    cosobj->u.n = strdup(name);

    *cosobj_out = cosobj;

    *offset_out = offset;

    return NSPDFERROR_OK;
}


/**
 * parse a COS boolean object
 */
static nspdferror
cos_parse_boolean(struct cos_stream *stream,
                  strmoff_t *offset_out,
                  struct cos_object **cosobj_out)
{
    strmoff_t offset;
    struct cos_object *cosobj;
    uint8_t c;
    bool value;

    offset = *offset_out;

    c = stream_byte(stream, offset++);
    if ((c == 't') || (c == 'T')) {
        /* true branch */

        c = stream_byte(stream, offset++);
        if ((c != 'r') && (c != 'R')) {
            return NSPDFERROR_SYNTAX;
        }

        c = stream_byte(stream, offset++);
        if ((c != 'u') && (c != 'U')) {
            return NSPDFERROR_SYNTAX;
        }

        c = stream_byte(stream, offset++);
        if ((c != 'e') && (c != 'E')) {
            return NSPDFERROR_SYNTAX;
        }
        value = true;

    } else if ((c == 'f') || (c == 'F')) {
        /* false branch */

        c = stream_byte(stream, offset++);
        if ((c != 'a') && (c != 'A')) {
            return NSPDFERROR_SYNTAX;
        }

        c = stream_byte(stream, offset++);
        if ((c != 'l') && (c != 'L')) {
            return NSPDFERROR_SYNTAX;
        }

        c = stream_byte(stream, offset++);
        if ((c != 's') && (c != 'S')) {
            return NSPDFERROR_SYNTAX;
        }

        c = stream_byte(stream, offset++);
        if ((c != 'e') && (c != 'E')) {
            return NSPDFERROR_SYNTAX;
        }

        value = false;

    } else {
        return NSPDFERROR_SYNTAX;
    }

    nspdf__stream_skip_ws(stream, &offset);

    cosobj = calloc(1, sizeof(struct cos_object));
    if (cosobj == NULL) {
        return NSPDFERROR_NOMEM; /* memory error */
    }

    cosobj->type = COS_TYPE_BOOL;
    cosobj->u.b = value;

    *cosobj_out = cosobj;

    *offset_out = offset;

    return NSPDFERROR_OK;
}


/**
 * parse a COS null object.
 */
static nspdferror
cos_parse_null(struct cos_stream *stream,
               strmoff_t *offset_out,
               struct cos_object **cosobj_out)
{
    strmoff_t offset;
    struct cos_object *cosobj;
    uint8_t c;

    offset = *offset_out;

    c = stream_byte(stream, offset++);
    if ((c != 'n') && (c != 'N')) {
        return NSPDFERROR_SYNTAX;
    }

    c = stream_byte(stream, offset++);
    if ((c != 'u') && (c != 'U')) {
        return NSPDFERROR_SYNTAX;
    }

    c = stream_byte(stream, offset++);
    if ((c != 'l') && (c != 'L')) {
        return NSPDFERROR_SYNTAX;
    }

    c = stream_byte(stream, offset++);
    if ((c != 'l') && (c != 'L')) {
        return NSPDFERROR_SYNTAX;
    }

    nspdf__stream_skip_ws(stream, &offset);

    cosobj = calloc(1, sizeof(struct cos_object));
    if (cosobj == NULL) {
        return NSPDFERROR_NOMEM;
    }

    cosobj->type = COS_TYPE_NULL;

    *cosobj_out = cosobj;

    *offset_out = offset;

    return NSPDFERROR_OK;
}

/**
 * parse a stream object
 */
static nspdferror
cos_parse_stream(struct nspdf_doc *doc,
                 struct cos_stream *stream_in,
                 strmoff_t *offset_out,
                 struct cos_object **cosobj_out)
{
    struct cos_object *cosobj;
    nspdferror res;
    struct cos_object *stream_dict;
    strmoff_t offset;
    struct cos_object *stream_filter;
    struct cos_stream *stream;
    int64_t stream_length;

    offset = *offset_out;
    stream_dict = *cosobj_out;

    if (stream_dict->type != COS_TYPE_DICTIONARY) {
        /* cannot be a stream if indirect object is not a dict */
        return NSPDFERROR_NOTFOUND;
    }

    if ((stream_byte(stream_in, offset    ) != 's') ||
        (stream_byte(stream_in, offset + 1) != 't') ||
        (stream_byte(stream_in, offset + 2) != 'r') ||
        (stream_byte(stream_in, offset + 3) != 'e') ||
        (stream_byte(stream_in, offset + 4) != 'a') ||
        (stream_byte(stream_in, offset + 5) != 'm')) {
        /* no stream marker */
        return NSPDFERROR_NOTFOUND;
    }
    offset += 6;
    //printf("detected stream\n");

    /* parsed object was a dictionary and there is a stream marker */
    res = nspdf__stream_skip_ws(stream_in, &offset);
    if (res != NSPDFERROR_OK) {
        return res;
    }

    stream = calloc(1, sizeof(struct cos_stream));
    if (stream == NULL) {
        return NSPDFERROR_NOMEM;
    }

    res = cos_get_dictionary_int(doc, stream_dict, "Length", &stream_length);
    if (res != NSPDFERROR_OK) {
        return res;
    }
    if (stream_length < 0) {
        return NSPDFERROR_RANGE;
    }
    stream->length = stream_length;

    //printf("stream length %d\n", stream_length);
    stream->data = stream_in->data + offset;
    stream->alloc = 0; /* stream is pointing at non malloced data */

    offset += stream->length;

    /* possible whitespace after stream data */
    res = nspdf__stream_skip_ws(stream_in, &offset);
    if (res != NSPDFERROR_OK) {
        return res;
    }

    if ((stream_byte(stream_in, offset    ) != 'e') ||
        (stream_byte(stream_in, offset + 1) != 'n') ||
        (stream_byte(stream_in, offset + 2) != 'd') ||
        (stream_byte(stream_in, offset + 3) != 's') ||
        (stream_byte(stream_in, offset + 4) != 't') ||
        (stream_byte(stream_in, offset + 5) != 'r') ||
        (stream_byte(stream_in, offset + 6) != 'e') ||
        (stream_byte(stream_in, offset + 7) != 'a') ||
        (stream_byte(stream_in, offset + 8) != 'm')) {
        /* no endstream marker */
        return NSPDFERROR_SYNTAX;
    }
    offset += 9;
    //printf("detected endstream\n");

    res = nspdf__stream_skip_ws(stream_in, &offset);
    if (res != NSPDFERROR_OK) {
        return res;
    }

    //printf("returning with offset at %d\n", offset);
    /* optional filter */
    res = cos_get_dictionary_value(doc, stream_dict, "Filter", &stream_filter);
    if (res == NSPDFERROR_OK) {
        const char *filter_name;
        res = cos_get_name(doc, stream_filter, &filter_name);
        if (res == NSPDFERROR_OK) {
            res = nspdf__cos_stream_filter(doc, filter_name, &stream);
            if (res != NSPDFERROR_OK) {
                return res;
            }
        } else {
            /** \todo array of filter stream */
        }
    }

    /* allocate stream object */
    cosobj = calloc(1, sizeof(struct cos_object));
    if (cosobj == NULL) {
        free(stream);
        return NSPDFERROR_NOMEM;
    }
    cosobj->type = COS_TYPE_STREAM;
    cosobj->u.stream = stream;

    *cosobj_out = cosobj;
    *offset_out = offset;

    return NSPDFERROR_OK;
}

/**
 * attempt to decode input data into a reference, indirect or stream object
 *
 * The input data already had a positive integer decoded from it:
 * - if another positive integer follows and a R character after that it is a
 *     reference,
 *
 * - if another positive integer follows and 'obj' after that:
 *     - a direct object followed by 'endobj' it is an indirect object.
 *
 *     - a direct dictionary object followed by 'stream', then stream data,
 *       then 'endstream' then 'endobj' it is a stream object
 *
 * \param doc the pdf document
 * \param offset_out offset of current cursor in input data
 * \param cosobj_out the object to return into, on input contains the first
 * integer
 */
static nspdferror
cos_attempt_parse_reference(struct nspdf_doc *doc,
                            struct cos_stream *stream,
                            strmoff_t *offset_out,
                            struct cos_object **cosobj_out)
{
    nspdferror res;
    strmoff_t offset;
    uint8_t c;
    struct cos_object *generation; /* generation object */

    offset = *offset_out;

    res = cos_parse_number(stream, &offset, &generation);
    if (res != NSPDFERROR_OK) {
        /* no error if next token could not be decoded as a number */
        return NSPDFERROR_OK;
    }

    if (generation->type != COS_TYPE_INT) {
        /* next object was not an integer so not a reference */
        cos_free_object(generation);
        return NSPDFERROR_OK;
    }

    if (generation->u.i < 0) {
        /* integer was negative so not a reference (generations must be
         * non-negative
         */
        cos_free_object(generation);
        return NSPDFERROR_OK;
    }

    /* two int in a row, look for the R */
    c = stream_byte(stream, offset);
    if (c == 'R') {
        struct cos_reference *nref; /* new reference */

        //printf("found object reference\n");
        offset ++;

        nspdf__stream_skip_ws(stream, &offset);

        nref = calloc(1, sizeof(struct cos_reference));
        if (nref == NULL) {
            cos_free_object(generation);
            return NSPDFERROR_NOMEM; /* memory error */
        }

        nref->id = (*cosobj_out)->u.i;
        nref->generation = generation->u.i;

        /* overwrite input object for output (it has to be an int which has no
         * allocation to free)
         */
        (*cosobj_out)->type = COS_TYPE_REFERENCE;
        (*cosobj_out)->u.reference = nref;

        *offset_out = offset;

    } else if ((c == 'o') &&
               (stream_byte(stream, offset + 1) == 'b') &&
               (stream_byte(stream, offset + 2) == 'j')) {
        struct cos_object *indirect; /* indirect object */
        //printf("indirect\n");
        offset += 3;

        res = nspdf__stream_skip_ws(stream, &offset);
        if (res != NSPDFERROR_OK) {
            cos_free_object(generation);
            return res;
        }
        //printf("decoding\n");

        res = cos_parse_object(doc, stream, &offset, &indirect);
        if (res != NSPDFERROR_OK) {
            cos_free_object(generation);
            return res;
        }

        /* attempt to parse input as a stream */
        res = cos_parse_stream(doc, stream, &offset, &indirect);
        if ((res != NSPDFERROR_OK) &&
            (res != NSPDFERROR_NOTFOUND)) {
            cos_free_object(indirect);
            cos_free_object(generation);
            return res;
        }

        /*printf("parsed indirect object num:%d gen:%d type %d\n",
               (*cosobj_out)->u.i,
               generation->u.i,
               indirect->type);
        */

        if ((stream_byte(stream, offset    ) != 'e') ||
            (stream_byte(stream, offset + 1) != 'n') ||
            (stream_byte(stream, offset + 2) != 'd') ||
            (stream_byte(stream, offset + 3) != 'o') ||
            (stream_byte(stream, offset + 4) != 'b') ||
            (stream_byte(stream, offset + 5) != 'j')) {
            cos_free_object(indirect);
            cos_free_object(generation);
            return NSPDFERROR_SYNTAX;
        }
        offset += 6;
        //printf("endobj\n");

        res = nspdf__stream_skip_ws(stream, &offset);
        if (res != NSPDFERROR_OK) {
            cos_free_object(indirect);
            cos_free_object(generation);
            return res;
        }

        cos_free_object(*cosobj_out);

        *cosobj_out = indirect;

        *offset_out = offset;

        //printf("returning object\n");
    }

    cos_free_object(generation);
    return NSPDFERROR_OK;
}


/*
 * Parse input stream into an object
 *
 * lex and parse a byte stream to generate COS objects
 *
 * lexing the input.
 *  check first character:
 *
 * < either a hex string or a dictionary
 *     second char < means dictionary else hex string
 * - either an integer or real
 * + either an integer or real
 * 0-9 an integer, unsigned integer or real
 * . a real number
 * ( a string
 * / a name
 * [ a list
 * t|T boolean true
 * f|F boolean false
 * n|N null
 *
 * Grammar is:
 * cos_object:
 *   TOK_NULL |
 *   TOK_BOOLEAN |
 *   TOK_INT |
 *   TOK_REAL |
 *   TOK_NAME |
 *   TOK_STRING |
 *   list |
 *   dictionary |
 *   object_reference |
 *   indirect_object;
 *
 * list:
 *   '[' listargs ']';
 *
 * listargs:
 *   cos_object
 *   |
 *   listargs cos_object
 *   ;
 *
 * object_reference:
 *   TOK_UINT TOK_UINT 'R';
 *
 * indirect_object:
 *   TOK_UINT TOK_UINT 'obj' cos_object 'endobj'
 *   |
 *  TOK_UINT TOK_UINT 'obj' dictionary 'stream' streamdata 'endstream' 'endobj'
 *   ;
 */
nspdferror
cos_parse_object(struct nspdf_doc *doc,
                 struct cos_stream *stream,
                 strmoff_t *offset_out,
                 struct cos_object **cosobj_out)
{
    strmoff_t offset;
    nspdferror res;
    struct cos_object *cosobj;

    offset = *offset_out;

    if (offset >= stream->length) {
        return NSPDFERROR_RANGE;
    }

    /* object could be any type use first char to try and select */
    switch (stream_byte(stream, offset)) {

    case '-': case '+': case '.': case '0': case '1': case '2': case '3':
    case '4': case '5': case '6': case '7': case '8': case '9':
        res = cos_parse_number(stream, &offset, &cosobj);
        /* if type is positive integer try to check for reference */
        if ((res == NSPDFERROR_OK) &&
            (cosobj->type == COS_TYPE_INT) &&
            (cosobj->u.i > 0)) {
            res = cos_attempt_parse_reference(doc, stream, &offset, &cosobj);
        }
        break;

    case 't':
    case 'f':
        res = cos_parse_boolean(stream, &offset, &cosobj);
        break;

    case 'n':
        res = cos_parse_null(stream, &offset, &cosobj);
        break;

    case '(':
        res = cos_parse_string(stream, &offset, &cosobj);
        break;

    case '/':
        res = cos_parse_name(stream, &offset, &cosobj);
        break;

    case '<':
        if (stream_byte(stream, offset + 1) == '<') {
            res = cos_parse_dictionary(doc, stream, &offset, &cosobj);
        } else {
            res = cos_parse_hex_string(stream, &offset, &cosobj);
        }
        break;

    case '[':
        res = cos_parse_list(doc, stream, &offset, &cosobj);
        break;

    default:
        res = NSPDFERROR_SYNTAX; /* syntax error */
    }

    if (res == NSPDFERROR_OK) {
        *cosobj_out = cosobj;
        *offset_out = offset;
    }

    return res;
}


static nspdferror
parse_operator(struct cos_stream *stream,
               strmoff_t *offset_out,
               enum content_operator *operator_out)
{
    nspdferror res;
    strmoff_t offset;
    enum content_operator operator;
    uint8_t c;
    unsigned int lookup;

    offset = *offset_out;

    /* first char */
    c = stream_byte(stream, offset);
    if ((bclass[c] & (BC_WSPC | BC_CMNT) ) != 0) {
        /* must have at least one non-whitespace character */
        return NSPDFERROR_SYNTAX;
    }
    lookup = c;
    offset++;
    /* possible second char */
    c = stream_byte(stream, offset);
    if ((bclass[c] & (BC_WSPC | BC_CMNT) ) == 0) {
        lookup = (lookup << 8) | c;
        offset++;

        /* possible third char */
        c = stream_byte(stream, offset);
        if ((bclass[c] & (BC_WSPC | BC_CMNT) ) == 0) {
            lookup = (lookup << 8) | c;
            offset++;

            /* fourth char must be whitespace */
            c = stream_byte(stream, offset);
            if ((bclass[c] & (BC_WSPC | BC_CMNT) ) == 0) {
                return NSPDFERROR_SYNTAX;
            }
        }
    }

    res = nspdf__stream_skip_ws(stream, &offset);
    if (res != NSPDFERROR_OK) {
        return res;
    }

    switch (lookup) {
    case '"': operator = CONTENT_OP___;
    case '\'': operator = CONTENT_OP__; break;
    case 'B': operator = CONTENT_OP_B; break;
    case 'F': operator = CONTENT_OP_F; break;
    case 'G': operator = CONTENT_OP_G; break;
    case 'J': operator = CONTENT_OP_J; break;
    case 'K': operator = CONTENT_OP_K; break;
    case 'M': operator = CONTENT_OP_M; break;
    case 'Q': operator = CONTENT_OP_Q; break;
    case 'S': operator = CONTENT_OP_S; break;
    case 'W': operator = CONTENT_OP_W; break;
    case 'b': operator = CONTENT_OP_b; break;
    case 'c': operator = CONTENT_OP_c; break;
    case 'd': operator = CONTENT_OP_d; break;
    case 'f': operator = CONTENT_OP_f; break;
    case 'g': operator = CONTENT_OP_g; break;
    case 'h': operator = CONTENT_OP_h; break;
    case 'i': operator = CONTENT_OP_i; break;
    case 'j': operator = CONTENT_OP_j; break;
    case 'k': operator = CONTENT_OP_k; break;
    case 'l': operator = CONTENT_OP_l; break;
    case 'm': operator = CONTENT_OP_m; break;
    case 'n': operator = CONTENT_OP_n; break;
    case 'q': operator = CONTENT_OP_q; break;
    case 's': operator = CONTENT_OP_s; break;
    case 'v': operator = CONTENT_OP_v; break;
    case 'w': operator = CONTENT_OP_w; break;
    case 'y': operator = CONTENT_OP_y; break;

    case (('B' << 8) | '*'): operator = CONTENT_OP_B_; break;
    case (('T' << 8) | '*'): operator = CONTENT_OP_T_; break;
    case (('W' << 8) | '*'): operator = CONTENT_OP_W_; break;

    case (('B' << 8) | 'I'): operator = CONTENT_OP_BI; break;
    case (('B' << 8) | 'T'): operator = CONTENT_OP_BT; break;
    case (('B' << 8) | 'X'): operator = CONTENT_OP_BX; break;
    case (('C' << 8) | 'S'): operator = CONTENT_OP_CS; break;
    case (('D' << 8) | 'P'): operator = CONTENT_OP_DP; break;
    case (('E' << 8) | 'I'): operator = CONTENT_OP_EI; break;
    case (('E' << 8) | 'T'): operator = CONTENT_OP_ET; break;
    case (('E' << 8) | 'X'): operator = CONTENT_OP_EX; break;
    case (('I' << 8) | 'D'): operator = CONTENT_OP_ID; break;
    case (('M' << 8) | 'P'): operator = CONTENT_OP_MP; break;
    case (('R' << 8) | 'G'): operator = CONTENT_OP_RG; break;
    case (('S' << 8) | 'S'): operator = CONTENT_OP_SC; break;
    case (('T' << 8) | 'D'): operator = CONTENT_OP_TD; break;
    case (('T' << 8) | 'J'): operator = CONTENT_OP_TJ; break;
    case (('T' << 8) | 'L'): operator = CONTENT_OP_TL; break;

    case (('D' << 8) | 'o'): operator = CONTENT_OP_Do; break;
    case (('T' << 8) | 'c'): operator = CONTENT_OP_Tc; break;
    case (('T' << 8) | 'd'): operator = CONTENT_OP_Td; break;
    case (('T' << 8) | 'f'): operator = CONTENT_OP_Tf; break;
    case (('T' << 8) | 'j'): operator = CONTENT_OP_Tj; break;
    case (('T' << 8) | 'm'): operator = CONTENT_OP_Tm; break;
    case (('T' << 8) | 'r'): operator = CONTENT_OP_Tr; break;
    case (('T' << 8) | 's'): operator = CONTENT_OP_Ts; break;
    case (('T' << 8) | 'w'): operator = CONTENT_OP_Tw; break;
    case (('T' << 8) | 'z'): operator = CONTENT_OP_Tz; break;

    case (('b' << 8) | '*'): operator = CONTENT_OP_b_; break;
    case (('f' << 8) | '*'): operator = CONTENT_OP_f_; break;
    case (('d' << 8) | '0'): operator = CONTENT_OP_d0; break;
    case (('d' << 8) | '1'): operator = CONTENT_OP_d1; break;

    case (('c' << 8) | 'm'): operator = CONTENT_OP_cm; break;
    case (('c' << 8) | 's'): operator = CONTENT_OP_cs; break;
    case (('g' << 8) | 's'): operator = CONTENT_OP_gs; break;
    case (('r' << 8) | 'e'): operator = CONTENT_OP_re; break;
    case (('r' << 8) | 'g'): operator = CONTENT_OP_rg; break;
    case (('r' << 8) | 'i'): operator = CONTENT_OP_ri; break;
    case (('s' << 8) | 'c'): operator = CONTENT_OP_sc; break;
    case (('s' << 8) | 'h'): operator = CONTENT_OP_sh; break;

    case (('B' << 16) | (('D' << 8) | 'C')): operator = CONTENT_OP_BDC; break;
    case (('B' << 16) | (('M' << 8) | 'C')): operator = CONTENT_OP_BMC; break;
    case (('E' << 16) | (('M' << 8) | 'C')): operator = CONTENT_OP_EMC; break;
    case (('S' << 16) | (('C' << 8) | 'N')): operator = CONTENT_OP_SCN; break;
    case (('s' << 16) | (('c' << 8) | 'n')): operator = CONTENT_OP_scn; break;

    default:
        return NSPDFERROR_SYNTAX;
    }

    *operator_out = operator;
    *offset_out = offset;

    return NSPDFERROR_OK;
}

#define MAX_OPERAND_COUNT 32

static nspdferror
parse_content_operation(struct nspdf_doc *doc,
                        struct cos_stream *stream,
                        strmoff_t *offset_out,
                        struct content_operation *operation_out)
{
    strmoff_t offset;
    nspdferror res;
    enum content_operator operator;
    struct cos_object *operands[MAX_OPERAND_COUNT];
    unsigned int operand_idx = 0;

    offset = *offset_out;

    res = parse_operator(stream, &offset, &operator);
    while (res == NSPDFERROR_SYNTAX) {
        /* was not an operator so check for what else it could have been */
        if (operand_idx >= MAX_OPERAND_COUNT) {
            /** \todo free any stacked operands */
            printf("too many operands\n");
            return NSPDFERROR_SYNTAX;
        }

        switch (stream_byte(stream, offset)) {

        case '-': case '+': case '.': case '0': case '1': case '2': case '3':
        case '4': case '5': case '6': case '7': case '8': case '9':
            res = cos_parse_number(stream, &offset, &operands[operand_idx]);
            break;

        case 't':
        case 'f':
            res = cos_parse_boolean(stream, &offset, &operands[operand_idx]);
            break;

        case 'n':
            res = cos_parse_null(stream, &offset, &operands[operand_idx]);
            break;

        case '(':
            res = cos_parse_string(stream, &offset, &operands[operand_idx]);
            break;

        case '/':
            res = cos_parse_name(stream, &offset, &operands[operand_idx]);
            break;

        case '[':
            res = cos_parse_list(doc, stream, &offset, &operands[operand_idx]);
            break;

        case '<':
            if (stream_byte(stream, offset + 1) == '<') {
                res = cos_parse_dictionary(doc,
                                           stream,
                                           &offset,
                                           &operands[operand_idx]);
            } else {
                res = cos_parse_hex_string(stream,
                                           &offset,
                                           &operands[operand_idx]);
            }
            break;

        default:
            printf("unknown operand type\n");
            res = NSPDFERROR_SYNTAX; /* syntax error */
        }

        if (res != NSPDFERROR_OK) {
            /* parse error */
            /** \todo free any stacked operands */
            printf("operand parse failed at %c\n",
                   stream_byte(stream, offset));
            return res;
        }

        /* move to next operand */
        operand_idx++;

        res = parse_operator(stream, &offset, &operator);
    }

    operation_out->operator = operator;
    //printf("returning operator %d with %d operands\n", operator, operand_idx);

    *offset_out = offset;
    return NSPDFERROR_OK;
}

nspdferror
cos_parse_content_stream(struct nspdf_doc *doc,
                         struct cos_stream *stream,
                         struct cos_object **content_out)
{
    nspdferror res;
    struct cos_object *cosobj;
    strmoff_t offset;

    //printf("%.*s", (int)stream->length, stream->data);

    cosobj = calloc(1, sizeof(struct cos_object));
    if (cosobj == NULL) {
        return NSPDFERROR_NOMEM;
    }
    cosobj->type = COS_TYPE_CONTENT;

    cosobj->u.content = calloc(1, sizeof (struct cos_content));
    if (cosobj->u.content == NULL) {
        res = NSPDFERROR_NOMEM;
        goto cos_parse_content_stream_error;
     }

    offset = 0;

    /* skip any leading whitespace */
    res = nspdf__stream_skip_ws(stream, &offset);
    if (res != NSPDFERROR_OK) {
        goto cos_parse_content_stream_error;
    }

    while (offset < stream->length) {
        struct content_operation cop;

        /* ensure there is space in the operations array */
        if (cosobj->u.content->alloc < (cosobj->u.content->length + 1)) {
            struct content_operation *newops;
            newops = realloc(cosobj->u.content->operations,
                             sizeof(struct content_operation) *
                             (cosobj->u.content->alloc + 32));
            if (newops == NULL) {
                res = NSPDFERROR_NOMEM;
                goto cos_parse_content_stream_error;
            }
            cosobj->u.content->operations = newops;
            cosobj->u.content->alloc += 32;
        }

        res = parse_content_operation(
            doc,
            stream,
            &offset,
            cosobj->u.content->operations + cosobj->u.content->length);
        if (res != NSPDFERROR_OK) {
            goto cos_parse_content_stream_error;
        }
        cosobj->u.content->length++;
    }

    *content_out = cosobj;

    return NSPDFERROR_OK;

cos_parse_content_stream_error:
    cos_free_object(cosobj);
    return res;
}
