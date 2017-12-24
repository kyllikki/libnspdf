#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "byte_class.h"
#include "nspdferror.h"
#include "cos_object.h"
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

static nspdferror
cos_decode_number(struct pdf_doc *doc,
                      uint64_t *offset_out,
                      struct cos_object **cosobj_out)
{
    struct cos_object *cosobj;
    uint8_t c; /* current byte from source data */
    unsigned int len; /* number of decimal places in number */
    uint8_t num[21]; /* temporary buffer for decimal values */
    uint64_t offset; /* current offset of source data */

    offset = *offset_out;

    for (len = 0; len < sizeof(num); len++) {
        c = DOC_BYTE(doc, offset);
        if ((bclass[c] & BC_DCML) != BC_DCML) {
            int64_t result = 0; /* parsed result */
            uint64_t tens;

            if (len == 0) {
                 /* parse error no decimals in input */
                return NSPDFERROR_SYNTAX;
            }
            /* sum value from each place */
            for (tens = 1; len > 0; tens = tens * 10, len--) {
                result += (num[len - 1] * tens);
            }

            doc_skip_ws(doc, &offset);

            cosobj = calloc(1, sizeof(struct cos_object));
            if (cosobj == NULL) {
                return NSPDFERROR_NOMEM;
            }

            cosobj->type = COS_TYPE_INT;
            cosobj->u.i = result;

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
 * decode literal string
 *
 */
static nspdferror
cos_decode_string(struct pdf_doc *doc,
                  uint64_t *offset_out,
                  struct cos_object **cosobj_out)
{
    uint64_t offset;
    struct cos_object *cosobj;
    uint8_t c;
    unsigned int pdepth = 1; /* depth of open parens */
    struct cos_string *cstring;

    offset = *offset_out;

    c = DOC_BYTE(doc, offset++);
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
        c = DOC_BYTE(doc, offset++);

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
            c = DOC_BYTE(doc, offset);
            while ((bclass[c] & BC_EOLM) != 0) {
                offset++;
                c = DOC_BYTE(doc, offset);
            }
            c = '\n';
        } else if (c == '\\') {
            /* escaped chars */
            c = DOC_BYTE(doc, offset++);
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
                    c = DOC_BYTE(doc, offset++);
                    while ((bclass[c] & BC_EOLM) != 0) {
                        c = DOC_BYTE(doc, offset++);
                    }
                } else if ((bclass[c] & BC_OCTL) != 0) {
                    /* octal value */
                    uint8_t val;
                    val = (c - '0');
                    c = DOC_BYTE(doc, offset);
                    if ((bclass[c] & BC_OCTL) != 0) {
                        offset++;
                        val = (val << 3) | (c - '0');
                        c = DOC_BYTE(doc, offset);
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

    doc_skip_ws(doc, &offset);

    *cosobj_out = cosobj;
    *offset_out = offset;

    return NSPDFERROR_OK;
}

/**
 * decode hex encoded string
 */
static nspdferror
cos_decode_hex_string(struct pdf_doc *doc,
                      uint64_t *offset_out,
                      struct cos_object **cosobj_out)
{
    uint64_t offset;
    struct cos_object *cosobj;
    uint8_t c;
    uint8_t value = 0;
    struct cos_string *cstring;
    bool first = true;

    offset = *offset_out;

    c = DOC_BYTE(doc, offset++);
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

    for (; offset < doc->length; offset++) {
        c = DOC_BYTE(doc, offset);
        if (c == '>') {
            if (first == false) {
                cos_string_append(cstring, value);
            }
            offset++;
            doc_skip_ws(doc, &offset);

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
 * decode a dictionary object
 */
static nspdferror
cos_decode_dictionary(struct pdf_doc *doc,
                      uint64_t *offset_out,
                      struct cos_object **cosobj_out)
{
    uint64_t offset;
    struct cos_object *cosobj;
    struct cos_dictionary_entry *entry;
    struct cos_object *key;
    struct cos_object *value;
    int res;

    offset = *offset_out;

    if ((DOC_BYTE(doc, offset) != '<') ||
        (DOC_BYTE(doc, offset + 1) != '<')) {
        return -1; /* syntax error */
    }
    offset += 2;
    doc_skip_ws(doc, &offset);

    //printf("found a dictionary\n");

    cosobj = calloc(1, sizeof(struct cos_object));
    if (cosobj == NULL) {
        return -1; /* memory error */
    }
    cosobj->type = COS_TYPE_DICTIONARY;

    while ((DOC_BYTE(doc, offset) != '>') &&
           (DOC_BYTE(doc, offset + 1) != '>')) {

        res = cos_decode_object(doc, &offset, &key);
        if (res != NSPDFERROR_OK) {
            /* todo free up any dictionary entries already created */
            printf("key object decode failed\n");
            return res;
        }
        if (key->type != COS_TYPE_NAME) {
            /* key value pairs without a name */
            printf("key was %d not a name %d\n", key->type, COS_TYPE_NAME);
            return NSPDFERROR_SYNTAX;
        }
        //printf("key: %s\n", key->u.n);

        res = cos_decode_object(doc, &offset, &value);
        if (res != NSPDFERROR_OK) {
            printf("Unable to decode value object in dictionary\n");
            /* todo free up any dictionary entries already created */
            return res;
        }

        /* add dictionary entry */
        entry = calloc(1, sizeof(struct cos_dictionary_entry));
        if (entry == NULL) {
            /* todo free up any dictionary entries already created */
            return NSPDFERROR_NOMEM;
        }

        entry->key = key;
        entry->value = value;
        entry->next = cosobj->u.dictionary;

        cosobj->u.dictionary = entry;

    }
    offset += 2; /* skip closing >> */
    doc_skip_ws(doc, &offset);

    *cosobj_out = cosobj;
    *offset_out = offset;

    return NSPDFERROR_OK;
}

/**
 * decode a list
 */
static nspdferror
cos_decode_list(struct pdf_doc *doc,
                uint64_t *offset_out,
                struct cos_object **cosobj_out)
{
    uint64_t offset;
    struct cos_object *cosobj;
    struct cos_array_entry *entry;
    struct cos_object *value;
    nspdferror res;

    offset = *offset_out;

    /* sanity check first token is list open */
    if (DOC_BYTE(doc, offset) != '[') {
        printf("not a [\n");
        return NSPDFERROR_SYNTAX; /* syntax error */
    }
    offset++;

    /* advance offset to next token */
    res = doc_skip_ws(doc, &offset);
    if (res != NSPDFERROR_OK) {
        return res;
    }

    printf("found a list\n");

    cosobj = calloc(1, sizeof(struct cos_object));
    if (cosobj == NULL) {
        return NSPDFERROR_NOMEM;
    }
    cosobj->type = COS_TYPE_ARRAY;

    while (DOC_BYTE(doc, offset) != ']') {

        res = cos_decode_object(doc, &offset, &value);
        if (res != NSPDFERROR_OK) {
            cos_free_object(cosobj);
            printf("Unable to decode value object in list\n");
            return res;
        }

        /* add entry to array */
        entry = calloc(1, sizeof(struct cos_array_entry));
        if (entry == NULL) {
            cos_free_object(cosobj);
            return NSPDFERROR_NOMEM;
        }

        entry->value = value;
        entry->next = cosobj->u.array;

        cosobj->u.array = entry;
    }
    offset++; /* skip closing ] */

    doc_skip_ws(doc, &offset);

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
cos_decode_name(struct pdf_doc *doc,
                uint64_t *offset_out,
                struct cos_object **cosobj_out)
{
    uint64_t offset;
    struct cos_object *cosobj;
    uint8_t c;
    char name[NAME_MAX_LENGTH + 1];
    int idx = 0;

    offset = *offset_out;

    c = DOC_BYTE(doc, offset++);
    if (c != '/') {
        return -1; /* names must be prefixed with a / */
    }
    //printf("found a name\n");

    c = DOC_BYTE(doc, offset);
    while ((idx <= NAME_MAX_LENGTH) &&
           ((bclass[c] & (BC_WSPC | BC_DELM)) == 0)) {
        offset++;
        //printf("%c", c);
        name[idx++] = c;
        c = DOC_BYTE(doc, offset);
    }
    //printf("\nidx: %d\n", idx);
    if (idx > NAME_MAX_LENGTH) {
        /* name length exceeded implementation limit */
        return -1;
    }
    name[idx] = 0;

    //printf("name: %s\n", name);

    doc_skip_ws(doc, &offset);

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
 * decode a cos boolean object
 */
static int
cos_decode_boolean(struct pdf_doc *doc,
                   uint64_t *offset_out,
                   struct cos_object **cosobj_out)
{
    uint64_t offset;
    struct cos_object *cosobj;
    uint8_t c;
    bool value;

    offset = *offset_out;

    c = DOC_BYTE(doc, offset++);
    if ((c == 't') || (c == 'T')) {
        /* true branch */

        c = DOC_BYTE(doc, offset++);
        if ((c != 'r') && (c != 'R')) {
            return -1; /* syntax error */
        }
        c = DOC_BYTE(doc, offset++);
        if ((c != 'u') && (c != 'U')) {
            return -1; /* syntax error */
        }
        c = DOC_BYTE(doc, offset++);
        if ((c != 'e') && (c != 'E')) {
            return -1; /* syntax error */
        }
        value = true;

    } else if ((c == 'f') || (c == 'F')) {
        /* false branch */

        c = DOC_BYTE(doc, offset++);
        if ((c != 'a') && (c != 'A')) {
            return -1; /* syntax error */
        }
        c = DOC_BYTE(doc, offset++);
        if ((c != 'l') && (c != 'L')) {
            return -1; /* syntax error */
        }
        c = DOC_BYTE(doc, offset++);
        if ((c != 's') && (c != 'S')) {
            return -1; /* syntax error */
        }
        c = DOC_BYTE(doc, offset++);
        if ((c != 'e') && (c != 'E')) {
            return -1; /* syntax error */
        }

        value = false;

    } else {
        return -1; /* syntax error */
    }

    doc_skip_ws(doc, &offset);

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
 * decode the null object.
 */
static nspdferror
cos_decode_null(struct pdf_doc *doc,
                uint64_t *offset_out,
                struct cos_object **cosobj_out)
{
    uint64_t offset;
    struct cos_object *cosobj;
    uint8_t c;

    offset = *offset_out;

    c = DOC_BYTE(doc, offset++);
    if ((c != 'n') && (c != 'N')) {
        return -1; /* syntax error */
    }

    c = DOC_BYTE(doc, offset++);
    if ((c != 'u') && (c != 'U')) {
        return -1; /* syntax error */
    }

    c = DOC_BYTE(doc, offset++);
    if ((c != 'l') && (c != 'L')) {
        return -1; /* syntax error */
    }

    c = DOC_BYTE(doc, offset++);
    if ((c != 'l') && (c != 'L')) {
        return -1; /* syntax error */
    }

    doc_skip_ws(doc, &offset);

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
cos_attempt_decode_reference(struct pdf_doc *doc,
                             uint64_t *offset_out,
                             struct cos_object **cosobj_out)
{
    nspdferror res;
    uint64_t offset;
    uint8_t c;
    struct cos_object *generation; /* generation object, reused for output */
    struct cos_reference *nref; /* new reference */

    offset = *offset_out;

    res = cos_decode_number(doc, &offset, &generation);
    if (res != 0) {
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
    c = DOC_BYTE(doc, offset++);
    if (c != 'R') {
        /* no R so not a reference */
        cos_free_object(generation);
        return NSPDFERROR_OK;
    }

    /* found reference */

    //printf("found reference\n");
    doc_skip_ws(doc, &offset);

    nref = calloc(1, sizeof(struct cos_reference));
    if (nref == NULL) {
        /** \todo free objects */
        return NSPDFERROR_NOMEM; /* memory error */
    }

    nref->id = (*cosobj_out)->u.i;
    nref->generation = generation->u.i;

    cos_free_object(*cosobj_out);

    generation->type = COS_TYPE_REFERENCE;
    generation->u.reference = nref;

    *cosobj_out = generation;

    *offset_out = offset;

    return NSPDFERROR_OK;
}


/*
 * Decode input stream into an object
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
cos_decode_object(struct pdf_doc *doc,
                  uint64_t *offset_out,
                  struct cos_object **cosobj_out)
{
    uint64_t offset;
    nspdferror res;
    struct cos_object *cosobj;

    offset = *offset_out;

    /* object could be any type use first char to try and select */
    switch (DOC_BYTE(doc, offset)) {

    case '-':
    case '+':
    case '.':
    case '0':
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9':
        res = cos_decode_number(doc, &offset, &cosobj);
        /* if type is positive integer try to check for reference */
        if ((res == 0) &&
            (cosobj->type == COS_TYPE_INT) &&
            (cosobj->u.i > 0)) {
            res = cos_attempt_decode_reference(doc, &offset, &cosobj);
        }
        break;

    case '<':
        if (DOC_BYTE(doc, offset + 1) == '<') {
            res = cos_decode_dictionary(doc, &offset, &cosobj);
        } else {
            res = cos_decode_hex_string(doc, &offset, &cosobj);
        }
        break;

    case '(':
        res = cos_decode_string(doc, &offset, &cosobj);
        break;

    case '/':
        res = cos_decode_name(doc, &offset, &cosobj);
        break;

    case '[':
        res = cos_decode_list(doc, &offset, &cosobj);
        break;

    case 't':
    case 'T':
    case 'f':
    case 'F':
        res = cos_decode_boolean(doc, &offset, &cosobj);
        break;

    case 'n':
    case 'N':
        res = cos_decode_null(doc, &offset, &cosobj);
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
