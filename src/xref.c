#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include "nspdferror.h"
#include "byte_class.h"
#include "cos_object.h"
#include "pdf_doc.h"

#define SLEN(x) (sizeof((x)) - 1)


int
read_whole_pdf(struct pdf_doc *doc, const char *fname)
{
    FILE *f;
    off_t len;
    uint8_t *buf;
    size_t rd;

    f = fopen(fname, "r");
    if (f == NULL) {
        perror("pdf open");
        return 1;
    }

    fseek(f, 0, SEEK_END);
    len = ftello(f);

    buf = malloc(len);
    fseek(f, 0, SEEK_SET);

    rd = fread(buf, len, 1, f);
    if (rd != 1) {
        perror("pdf read");
        free(buf);
        return 1;
    }

    fclose(f);

    doc->start = doc->buffer = buf;
    doc->length = doc->buffer_length = len;

    return 0;
}


#define STARTXREF_TOK "startxref"
/* Number of bytes to search back from file end to find xref start token, convention says 1024 bytes */
#define STARTXREF_SEARCH_SIZE 1024




static nspdferror
doc_read_uint(struct pdf_doc *doc, uint64_t *offset_out, uint64_t *result_out)
{
    uint8_t c; /* current byte from source data */
    unsigned int len; /* number of decimal places in number */
    uint8_t num[21]; /* temporary buffer for decimal values */
    uint64_t offset; /* current offset of source data */
    uint64_t result=0; /* parsed result */
    uint64_t tens;

    offset = *offset_out;

    for (len = 0; len < sizeof(num); len++) {
        c = DOC_BYTE(doc, offset);
        if ((bclass[c] & BC_DCML) != BC_DCML) {
            if (len == 0) {
                return -2; /* parse error no decimals in input */
            }
            /* sum value from each place */
            for (tens = 1; len > 0; tens = tens * 10, len--) {
                result += (num[len - 1] * tens);
            }

            *offset_out = offset;
            *result_out = result;

            return NSPDFERROR_OK;
        }
        num[len] = c - '0';
        offset++;
    }
    return -1; /* number too long */
}

/**
 * finds the startxref marker at the end of input
 */
nspdferror find_startxref(struct pdf_doc *doc, uint64_t *offset_out)
{
    uint64_t offset; /* offset of characters being considered for startxref */
    uint64_t earliest; /* earliest offset to serch for startxref */

    offset = doc->length - SLEN(STARTXREF_TOK);

    if (doc->length < STARTXREF_SEARCH_SIZE) {
        earliest = 0;
    } else {
        earliest = doc->length - STARTXREF_SEARCH_SIZE;
    }

    for (;offset > earliest; offset--) {
        if ((DOC_BYTE(doc, offset    ) == 's') &&
            (DOC_BYTE(doc, offset + 1) == 't') &&
            (DOC_BYTE(doc, offset + 2) == 'a') &&
            (DOC_BYTE(doc, offset + 3) == 'r') &&
            (DOC_BYTE(doc, offset + 4) == 't') &&
            (DOC_BYTE(doc, offset + 5) == 'x') &&
            (DOC_BYTE(doc, offset + 6) == 'r') &&
            (DOC_BYTE(doc, offset + 7) == 'e') &&
            (DOC_BYTE(doc, offset + 8) == 'f')) {
            *offset_out = offset;
            return NSPDFERROR_OK;
        }
    }
    return NSPDFERROR_SYNTAX;
}

/**
 * decodes a startxref field
 */
nspdferror decode_startxref(struct pdf_doc *doc, uint64_t *offset_out, uint64_t *start_xref_out)
{
    uint64_t offset; /* offset of characters being considered for startxref */
    uint64_t start_xref;
    nspdferror res;

    offset = *offset_out;

    if ((DOC_BYTE(doc, offset    ) != 's') ||
        (DOC_BYTE(doc, offset + 1) != 't') ||
        (DOC_BYTE(doc, offset + 2) != 'a') ||
        (DOC_BYTE(doc, offset + 3) != 'r') ||
        (DOC_BYTE(doc, offset + 4) != 't') ||
        (DOC_BYTE(doc, offset + 5) != 'x') ||
        (DOC_BYTE(doc, offset + 6) != 'r') ||
        (DOC_BYTE(doc, offset + 7) != 'e') ||
        (DOC_BYTE(doc, offset + 8) != 'f')) {
        return NSPDFERROR_SYNTAX;
    }
    offset += 9;

    res = doc_skip_ws(doc, &offset);
    if (res != NSPDFERROR_OK) {
        return res;
    }

    res = doc_read_uint(doc, &offset, &start_xref);
    if (res != NSPDFERROR_OK) {
        return res;
    }

    res = doc_skip_eol(doc, &offset);
    if (res != NSPDFERROR_OK) {
        return res;
    }

    if ((DOC_BYTE(doc, offset    ) != '%') ||
        (DOC_BYTE(doc, offset + 1) != '%') ||
        (DOC_BYTE(doc, offset + 2) != 'E') ||
        (DOC_BYTE(doc, offset + 3) != 'O') ||
        (DOC_BYTE(doc, offset + 4) != 'F')) {
        printf("missing EOF marker\n");
        return NSPDFERROR_SYNTAX;
    }

    *offset_out = offset;
    *start_xref_out = start_xref;

    return NSPDFERROR_OK;
}


/**
 * finds the next trailer
 */
nspdferror find_trailer(struct pdf_doc *doc, uint64_t *offset_out)
{
    uint64_t offset; /* offset of characters being considered for trailer */

    for (offset = *offset_out;offset < doc->length; offset++) {
        if ((DOC_BYTE(doc, offset    ) == 't') &&
            (DOC_BYTE(doc, offset + 1) == 'r') &&
            (DOC_BYTE(doc, offset + 2) == 'a') &&
            (DOC_BYTE(doc, offset + 3) == 'i') &&
            (DOC_BYTE(doc, offset + 4) == 'l') &&
            (DOC_BYTE(doc, offset + 5) == 'e') &&
            (DOC_BYTE(doc, offset + 6) == 'r')) {
            *offset_out = offset;
            return NSPDFERROR_OK;
        }
    }
    return NSPDFERROR_SYNTAX;
}

/**
 * find the PDF comment marker to identify the start of the document
 */
int check_header(struct pdf_doc *doc)
{
    uint64_t offset; /* offset of characters being considered for startxref */

    for (offset = 0; offset < 1024; offset++) {
        if ((DOC_BYTE(doc, offset) == '%') &&
            (DOC_BYTE(doc, offset + 1) == 'P') &&
            (DOC_BYTE(doc, offset + 2) == 'D') &&
            (DOC_BYTE(doc, offset + 3) == 'F') &&
            (DOC_BYTE(doc, offset + 4) == '-') &&
            (DOC_BYTE(doc, offset + 5) == '1') &&
            (DOC_BYTE(doc, offset + 6) == '.')) {
            doc->start = doc->buffer + offset;
            doc->length -= offset;
            /* read number for minor */
            return 0;
        }
    }
    return -1;
}






nspdferror
decode_trailer(struct pdf_doc *doc,
               uint64_t *offset_out,
               struct cos_object **trailer_out)
{
    struct cos_object *trailer;
    int res;
    uint64_t offset;

    offset = *offset_out;

    /* trailer object header */
    if ((DOC_BYTE(doc, offset    ) != 't') &&
        (DOC_BYTE(doc, offset + 1) != 'r') &&
        (DOC_BYTE(doc, offset + 2) != 'a') &&
        (DOC_BYTE(doc, offset + 3) != 'i') &&
        (DOC_BYTE(doc, offset + 4) != 'l') &&
        (DOC_BYTE(doc, offset + 5) != 'e') &&
        (DOC_BYTE(doc, offset + 6) != 'r')) {
        return -1;
    }
    offset += 7;
    doc_skip_ws(doc, &offset);

    res = cos_decode_object(doc, &offset, &trailer);
    if (res != 0) {
        return res;
    }

    if (trailer->type != COS_TYPE_DICTIONARY) {
        cos_free_object(trailer);
        return -1;
    }

    *trailer_out = trailer;
    *offset_out = offset;

    return NSPDFERROR_OK;
}

nspdferror
decode_xref(struct pdf_doc *doc, uint64_t *offset_out)
{
    uint64_t offset;
    nspdferror res;
    uint64_t objnumber; /* current object number */
    uint64_t objcount;

    offset = *offset_out;

    /* xref object header */
    if ((DOC_BYTE(doc, offset    ) != 'x') &&
        (DOC_BYTE(doc, offset + 1) != 'r') &&
        (DOC_BYTE(doc, offset + 2) != 'e') &&
        (DOC_BYTE(doc, offset + 3) != 'f')) {
        return NSPDFERROR_SYNTAX;
    }
    offset += 4;

    res = doc_skip_ws(doc, &offset);
    if (res != NSPDFERROR_OK) {
        return res;
    }

    /* subsections
     * <first object number> <number of references in subsection>
     */
    res = doc_read_uint(doc, &offset, &objnumber);
    while (res == NSPDFERROR_OK) {
        uint64_t lastobj;
        res = doc_skip_ws(doc, &offset);
        if (res != NSPDFERROR_OK) {
            return res;
        }

        res = doc_read_uint(doc, &offset, &objcount);
        if (res != NSPDFERROR_OK) {
            return res;
        }

        res = doc_skip_ws(doc, &offset);
        if (res != NSPDFERROR_OK) {
            return res;
        }

        //printf("decoding subsection %lld %lld\n", objnumber, objcount);

        lastobj = objnumber + objcount;
        for (; objnumber < lastobj ; objnumber++) {
            /* each entry is a fixed format */
            uint64_t objindex;
            uint64_t objgeneration;

            /* object index */
            res = doc_read_uint(doc, &offset, &objindex);
            if (res != NSPDFERROR_OK) {
                return res;
            }
            offset++; /* skip space */

            res = doc_read_uint(doc, &offset, &objgeneration);
            if (res != NSPDFERROR_OK) {
                return res;
            }
            offset++; /* skip space */

            if ((DOC_BYTE(doc, offset++) == 'n')) {
                if (objnumber < doc->xref_size) {
                    struct xref_table_entry *indobj;
                    indobj = doc->xref_table + objnumber;

                    indobj->ref.id = objnumber;
                    indobj->ref.generation = objgeneration;
                    indobj->offset = objindex;

                    //printf("xref %lld %lld -> %lld\n", objnumber, objgeneration, objindex);
                } else {
                    printf("index out of bounds\n");
                }
            }

            offset += 2; /* skip EOL */
        }

        res = doc_read_uint(doc, &offset, &objnumber);
    }

    return NSPDFERROR_OK;
}


/**
 * recursively parse trailers and xref tables
 */
nspdferror decode_xref_trailer(struct pdf_doc *doc, uint64_t xref_offset)
{
    nspdferror res;
    uint64_t offset; /* the current data offset */
    uint64_t startxref; /* the value of the startxref field */
    struct cos_object *trailer; /* the current trailer */
    struct cos_object *cobj_prev;
    int64_t prev;

    offset = xref_offset;

    res = find_trailer(doc, &offset);
    if (res != NSPDFERROR_OK) {
        printf("failed to find last trailer\n");
        return res;
    }

    res = decode_trailer(doc, &offset, &trailer);
    if (res != NSPDFERROR_OK) {
        printf("failed to decode trailer\n");
        return res;
    }

    res = decode_startxref(doc, &offset, &startxref);
    if (res != NSPDFERROR_OK) {
        printf("failed to decode startxref\n");
        goto decode_xref_trailer_failed;
    }

    if (startxref != xref_offset) {
        printf("startxref and Prev value disagree\n");
    }

    if (doc->xref_table == NULL) {
        /* extract Size from trailer and create xref table large enough */
        struct cos_object *cobj_size;
        int64_t size;

        res = cos_dictionary_get_value(trailer, "Size", &cobj_size);
        if (res != NSPDFERROR_OK) {
            printf("trailer has no Size value\n");
            goto decode_xref_trailer_failed;
        }

        res = cos_get_int(cobj_size, &size);
        if (res != NSPDFERROR_OK) {
            printf("trailer Size not int\n");
            goto decode_xref_trailer_failed;
        }

        res = cos_dictionary_extract_value(trailer, "Root", &doc->root);
        if (res != NSPDFERROR_OK) {
            printf("no Root!\n");
            goto decode_xref_trailer_failed;
        }

        doc->xref_table = calloc(size, sizeof(struct xref_table_entry));
        if (doc->xref_table == NULL) {
            res = NSPDFERROR_NOMEM;
            goto decode_xref_trailer_failed;
        }
        doc->xref_size = size;

        res = cos_dictionary_extract_value(trailer, "Encrypt", &doc->encrypt);
        if ((res != NSPDFERROR_OK) && (res != NSPDFERROR_NOTFOUND)) {
            goto decode_xref_trailer_failed;
        }

        res = cos_dictionary_extract_value(trailer, "Info", &doc->info);
        if ((res != NSPDFERROR_OK) && (res != NSPDFERROR_NOTFOUND)) {
            goto decode_xref_trailer_failed;
        }

        res = cos_dictionary_extract_value(trailer, "ID", &doc->id);
        if ((res != NSPDFERROR_OK) && (res != NSPDFERROR_NOTFOUND)) {
            goto decode_xref_trailer_failed;
        }

    }

    /* check for prev ID key in trailer and recurse call if present */
    res = cos_dictionary_get_value(trailer, "Prev", &cobj_prev);
    if (res == NSPDFERROR_OK) {
        res = cos_get_int(cobj_prev, &prev);
        if (res != NSPDFERROR_OK) {
            printf("trailer Prev not int\n");
            goto decode_xref_trailer_failed;
        }

        res = decode_xref_trailer(doc, prev);
        if (res != NSPDFERROR_OK) {
            goto decode_xref_trailer_failed;
        }
    }

    offset = xref_offset;
    /** @todo deal with XrefStm (number) in trailer */

    res = decode_xref(doc, &offset);
    if (res != NSPDFERROR_OK) {
        printf("failed to decode xref table\n");
        goto decode_xref_trailer_failed;
    }

decode_xref_trailer_failed:
    cos_free_object(trailer);

    return res;
}

/**
 * decode non-linear pdf trailer data
 *
 * PDF have a structure nominally defined as header, body, cross reference table
 * and trailer. The body, cross reference table and trailer sections may be
 * repeated in a scheme known as "incremental updates"
 *
 * The strategy used here is to locate the end of the last trailer block which
 * contains a startxref token followed by a byte offset into the file of the
 * beginning of the cross reference table followed by a literal '%%EOF'
 *
 * the initial offset is used to walk back down a chain of xref/trailers until
 * the trailer does not contain a Prev entry and decode xref tables forwards to
 * overwrite earlier object entries with later ones.
 *
 * It is necessary to search forwards from the xref table to find the trailer
 * block because instead of the Prev entry pointing to the previous trailer
 * (from which we could have extracted the startxref to find the associated
 * xref table) it points to the previous xref block which we have to skip to
 * find the subsequent trailer.
 *
 */
nspdferror decode_trailers(struct pdf_doc *doc)
{
    nspdferror res;
    uint64_t offset; /* the current data offset */
    uint64_t startxref; /* the value of the first startxref field */

    res = find_startxref(doc, &offset);
    if (res != NSPDFERROR_OK) {
        printf("failed to find startxref\n");
        return res;
    }

    res = decode_startxref(doc, &offset, &startxref);
    if (res != NSPDFERROR_OK) {
        printf("failed to decode startxref\n");
        return res;
    }

    /* recurse down the xref and trailers */
    return decode_xref_trailer(doc, startxref);
}

nspdferror decode_catalog(struct pdf_doc *doc)
{
    nspdferror res;
    struct cos_object *catalog;

    res = cos_get_dictionary(doc->root, &catalog);
    
    return res;
}

nspdferror new_pdf_doc(struct pdf_doc **doc_out)
{
    struct pdf_doc *doc;
    doc = calloc(1, sizeof(struct pdf_doc));
    if (doc == NULL) {
        return NSPDFERROR_NOMEM;
    }
    *doc_out = doc;
    return NSPDFERROR_OK;
}

int main(int argc, char **argv)
{
    struct pdf_doc *doc;
    int res;

    res = new_pdf_doc(&doc);
    if (res != NSPDFERROR_OK) {
        printf("failed to read file\n");
        return res;
    }

    res = read_whole_pdf(doc, argv[1]);
    if (res != 0) {
        printf("failed to read file\n");
        return res;
    }

    res = check_header(doc);
    if (res != 0) {
        printf("header check failed\n");
        return res;
    }

    res = decode_trailers(doc);
    if (res != NSPDFERROR_OK) {
        printf("failed to decode trailers (%d)\n", res);
        return res;
    }

    res = decode_catalog(doc);
    if (res != NSPDFERROR_OK) {
        printf("failed to decode catalog (%d)\n", res);
        return res;
    }

    return 0;
}
