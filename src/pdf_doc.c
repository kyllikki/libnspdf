
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#include "nspdferror.h"
#include "byte_class.h"
#include "cos_object.h"
#include "pdf_doc.h"

/**
 * move offset to next non whitespace byte
 */
int doc_skip_ws(struct pdf_doc *doc, uint64_t *offset)
{
    uint8_t c;
    /* TODO sort out keeping offset in range */
    c = DOC_BYTE(doc, *offset);
    while ((bclass[c] & (BC_WSPC | BC_CMNT) ) != 0) {
        (*offset)++;
        /* skip comments */
        if ((bclass[c] & BC_CMNT) != 0) {
            c = DOC_BYTE(doc, *offset);
            while ((bclass[c] & BC_EOLM ) == 0) {
                (*offset)++;
                c = DOC_BYTE(doc, *offset);
            }
        }
        c = DOC_BYTE(doc, *offset);
    }
    return 0;
}

/**
 * move offset to next non eol byte
 */
int doc_skip_eol(struct pdf_doc *doc, uint64_t *offset)
{
    uint8_t c;
    /* TODO sort out keeping offset in range */
    c = DOC_BYTE(doc, *offset);
    while ((bclass[c] & BC_EOLM) != 0) {
        (*offset)++;
        c = DOC_BYTE(doc, *offset);
    }
    return 0;
}
