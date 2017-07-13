#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#define SLEN(x) (sizeof((x)) - 1)

#define BC_NONE 0
#define BC_WSPC 1 /* character is whitespace */
#define BC_EOLM (1<<1) /* character signifies end of line */
#define BC_DCML (1<<2) /* character is a decimal */
#define BC_HEXL (1<<3) /* character is a hexadecimal */
#define BC_DELM (1<<4) /* character is a delimiter */

/**
 * pdf byte classification
 */
uint8_t bclass[] = {
    BC_WSPC, BC_NONE, BC_NONE, BC_NONE, /* 00 - 03 */
    BC_NONE, BC_NONE, BC_NONE, BC_NONE, /* 04 - 07 */
    BC_NONE, BC_WSPC, BC_WSPC | BC_EOLM, BC_NONE, /* 08 - 0B */
    BC_WSPC, BC_WSPC | BC_EOLM, BC_NONE, BC_NONE, /* 0C - 0F */
    BC_NONE, BC_NONE, BC_NONE, BC_NONE, /* 10 - 13 */
    BC_NONE, BC_NONE, BC_NONE, BC_NONE, /* 14 - 17 */
    BC_NONE, BC_NONE, BC_NONE, BC_NONE, /* 18 - 1B */
    BC_NONE, BC_NONE, BC_NONE, BC_NONE, /* 1C - 1F */
    BC_WSPC, BC_NONE, BC_NONE, BC_NONE, /* 20 - 23 */
    BC_NONE, BC_DELM,                     /* '$' '%' */
    BC_NONE, BC_NONE,                     /* 26 - 27 */
    BC_DELM, BC_DELM,                     /* '(' ')' */
    BC_NONE, BC_NONE,                     /* 2A - 2B */
    BC_NONE, BC_NONE,                     /* 2C - 2D */
    BC_NONE, BC_DELM,                     /* '.' '/' */
    BC_DCML | BC_HEXL, BC_DCML | BC_HEXL, /* '0' '1' */
    BC_DCML | BC_HEXL, BC_DCML | BC_HEXL, /* '2' '3' */
    BC_DCML | BC_HEXL, BC_DCML | BC_HEXL, /* '4' '5' */
    BC_DCML | BC_HEXL, BC_DCML | BC_HEXL, /* '6' '7' */
    BC_DCML | BC_HEXL, BC_DCML | BC_HEXL, /* '8' '9' */
    BC_NONE, BC_NONE,                     /* ':' ';' */
    BC_DELM, BC_NONE,                     /* '<' '=' */
    BC_DELM, BC_NONE,                     /* '>' '?' */
    BC_NONE, BC_NONE, BC_NONE, BC_NONE, /* 40 - 43 */
    BC_NONE, BC_HEXL, BC_HEXL, BC_HEXL, /* 44 - 47 */
    BC_HEXL, BC_HEXL, BC_HEXL, BC_NONE, /* 48 - 4B */
    BC_NONE, BC_NONE, BC_NONE, BC_NONE, /* 4C - 4F */
    BC_NONE, BC_NONE, BC_NONE, BC_NONE, /* 50 - 53 */
    BC_NONE, BC_NONE, BC_NONE, BC_NONE, /* 54 - 57 */
    BC_NONE, BC_NONE,                   /* 58 - 59 */
    BC_NONE, BC_DELM,                   /* 'Z' '[' */
    BC_NONE, BC_DELM,                   /* '\' ']' */
    BC_NONE, BC_NONE,                   /* 5E - 5F */
    BC_NONE, BC_NONE, BC_NONE, BC_NONE,
    BC_NONE, BC_HEXL, BC_HEXL, BC_HEXL, /* 60 - 67 */
    BC_HEXL, BC_HEXL, BC_HEXL, BC_NONE,
    BC_NONE, BC_NONE, BC_NONE, BC_NONE, /* 68 - 6F */
    BC_NONE, BC_NONE, BC_NONE, BC_NONE,
    BC_NONE, BC_NONE, BC_NONE, BC_NONE, /* 70 - 77 */
    BC_NONE, BC_NONE,                   /* 78 - 79 */
    BC_NONE, BC_DELM,                   /* 'z' '{' */
    BC_NONE, BC_DELM,                   /* '|' '}' */
    BC_NONE, BC_NONE,                   /* 7E - 7F */
    BC_NONE, BC_NONE, BC_NONE, BC_NONE, /* 80 - 83 */
    BC_NONE, BC_NONE, BC_NONE, BC_NONE, /* 84 - 87 */
    BC_NONE, BC_NONE, BC_NONE, BC_NONE,
    BC_NONE, BC_NONE, BC_NONE, BC_NONE, /* 88 - 8F */
    BC_NONE, BC_NONE, BC_NONE, BC_NONE,
    BC_NONE, BC_NONE, BC_NONE, BC_NONE, /* 90 - 97 */
    BC_NONE, BC_NONE, BC_NONE, BC_NONE,
    BC_NONE, BC_NONE, BC_NONE, BC_NONE, /* 98 - 9F */
    BC_NONE, BC_NONE, BC_NONE, BC_NONE,
    BC_NONE, BC_NONE, BC_NONE, BC_NONE, /* A0 - A7 */
    BC_NONE, BC_NONE, BC_NONE, BC_NONE,
    BC_NONE, BC_NONE, BC_NONE, BC_NONE, /* A8 - AF */
    BC_NONE, BC_NONE, BC_NONE, BC_NONE,
    BC_NONE, BC_NONE, BC_NONE, BC_NONE, /* B0 - B7 */
    BC_NONE, BC_NONE, BC_NONE, BC_NONE,
    BC_NONE, BC_NONE, BC_NONE, BC_NONE, /* B8 - BF */
    BC_NONE, BC_NONE, BC_NONE, BC_NONE,
    BC_NONE, BC_NONE, BC_NONE, BC_NONE, /* C0 - C7 */
    BC_NONE, BC_NONE, BC_NONE, BC_NONE,
    BC_NONE, BC_NONE, BC_NONE, BC_NONE, /* C8 - CF */
    BC_NONE, BC_NONE, BC_NONE, BC_NONE,
    BC_NONE, BC_NONE, BC_NONE, BC_NONE, /* D0 - D7 */
    BC_NONE, BC_NONE, BC_NONE, BC_NONE,
    BC_NONE, BC_NONE, BC_NONE, BC_NONE, /* D8 - DF */
    BC_NONE, BC_NONE, BC_NONE, BC_NONE,
    BC_NONE, BC_NONE, BC_NONE, BC_NONE, /* E0 - E7 */
    BC_NONE, BC_NONE, BC_NONE, BC_NONE,
    BC_NONE, BC_NONE, BC_NONE, BC_NONE, /* E8 - EF */
    BC_NONE, BC_NONE, BC_NONE, BC_NONE,
    BC_NONE, BC_NONE, BC_NONE, BC_NONE, /* F0 - F7 */
    BC_NONE, BC_NONE, BC_NONE, BC_NONE,
    BC_NONE, BC_NONE, BC_NONE, BC_NONE, /* F8 - FF */
};

enum cos_type {
    COS_TYPE_NULL,
    COS_TYPE_BOOL,
    COS_TYPE_INT,
    COS_TYPE_REAL,
    COS_TYPE_NAME,
    COS_TYPE_STRING,
    COS_TYPE_ARRAY,
    COS_TYPE_DICTIONARY,
    COS_TYPE_NAMETREE,
    COS_TYPE_NUMBERTREE,
    COS_TYPE_STREAM,
    COS_TYPE_REFERENCE,
};

struct cos_object;

struct cos_dictionary_entry {
    /** next key/value in dictionary */
    struct cos_dictionary_entry *next;

    /** key (name) */
    struct cos_object *key;

    /** value */
    struct cos_object *value;
};


struct cos_reference {
    /** id of indirect object */
    uint64_t id;

    /* generation of indirect object */
    uint64_t generation;
};

struct cos_object {
    int type;
    union {
        /** boolean */
        bool b;

        /** integer */
        int64_t i;

        /** real */
        double r;

        /** name */
        char *n;

        /** string */
        char *s;

        /** stream data */
        uint8_t *stream;

        /* dictionary */
        struct cos_dictionary_entry *dictionary;

        /** reference */
        struct cos_reference *reference;

    } u;
};


/** linked list of indirect objects */
struct cos_indirect_object {
    /** next in list */
    struct cos_indirect_object *next;

    /* reference identifier */
    struct cos_reference ref;

    /** offset of object */
    uint64_t offset;

    /* direct object */
    struct cos_object *o;
};

/** pdf document */
struct pdf_doc {
    uint8_t *buffer;
    uint64_t buffer_length;

    uint8_t *start; /* start of pdf document in input stream */
    uint64_t length;

    int major;
    int minor;

    /** start of current xref table  */
    uint64_t startxref;

    /** indirect objects from document body */
    struct cos_indirect_object *cos_list;
};


int cos_decode_object(struct pdf_doc *doc,
                      uint64_t *offset_out,
                      struct cos_object **cosobj_out);

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


/* byte data acessory, allows for more complex buffer handling in future */
#define DOC_BYTE(doc, offset) (doc->start[(offset)])

/* find next non whitespace byte */
static int doc_skip_ws(struct pdf_doc *doc, uint64_t *offset)
{
    uint8_t c;

    c = DOC_BYTE(doc, *offset);
    while ((bclass[c] & BC_WSPC) != 0) {
        (*offset)++;
        c = DOC_BYTE(doc, *offset);
    }
    return 0;
}

static int doc_read_uint(struct pdf_doc *doc, uint64_t *offset_out, uint64_t *result_out)
{
    uint8_t c; /* current byte from source data */
    int len; /* number of decimal places in number */
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

            return 0;
        }
        num[len] = c - '0';
        offset++;
    }
    return -1; /* number too long */
}

/**
 * finds the startxref marker at the end of input
 */
int find_startxref(struct pdf_doc *doc, uint64_t *start_xref_out)
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
            offset += 9;
            doc_skip_ws(doc, &offset);
            return doc_read_uint(doc, &offset, start_xref_out);
        }
    }
    return -1;
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

/* add indirect object */
int cos_indirect_object_add(struct pdf_doc *doc,
                    uint64_t obj_number,
                    uint64_t obj_offset,
                    uint64_t obj_generation)
{
    struct cos_indirect_object *nobj;
    nobj = calloc(1, sizeof(struct cos_indirect_object));

    if (nobj == NULL) {
        return -1;
    }
    nobj->next = doc->cos_list;
    nobj->ref.id = obj_number;
    nobj->ref.generation = obj_generation;
    nobj->offset = obj_offset;

    doc->cos_list = nobj;

    printf("xref %"PRIu64" %"PRIu64" %"PRIu64"\n",
           obj_number, obj_offset, obj_generation);
    return 0;
}

int cos_free_object(struct cos_object *cos_obj)
{
    struct cos_dictionary_entry *dentry;

    switch (cos_obj->type) {
    case COS_TYPE_NAME:
        free(cos_obj->u.n);
        break;

    case COS_TYPE_STRING:
        free(cos_obj->u.s);
        break;

    case COS_TYPE_DICTIONARY:
        dentry = cos_obj->u.dictionary;
        while (dentry != NULL) {
            struct cos_dictionary_entry *odentry;

            cos_free_object(dentry->key);
            cos_free_object(dentry->value);

            odentry = dentry;
            dentry = dentry->next;
            free(odentry);
        }
        break;

    case COS_TYPE_STREAM:
        free(cos_obj->u.stream);
        break;

    }
    free(cos_obj);

    return 0;
}

int cos_decode_number(struct pdf_doc *doc,
                      uint64_t *offset_out,
                      struct cos_object **cosobj_out)
{
    struct cos_object *cosobj;
    uint8_t c; /* current byte from source data */
    int len; /* number of decimal places in number */
    uint8_t num[21]; /* temporary buffer for decimal values */
    uint64_t offset; /* current offset of source data */

    offset = *offset_out;

    for (len = 0; len < sizeof(num); len++) {
        c = DOC_BYTE(doc, offset);
        if ((bclass[c] & BC_DCML) != BC_DCML) {
            int64_t result = 0; /* parsed result */
            uint64_t tens;

            if (len == 0) {
                return -2; /* parse error no decimals in input */
            }
            /* sum value from each place */
            for (tens = 1; len > 0; tens = tens * 10, len--) {
                result += (num[len - 1] * tens);
            }

            doc_skip_ws(doc, &offset);

            cosobj = calloc(1, sizeof(struct cos_object));
            if (cosobj == NULL) {
                return -1; /* memory error */
            }

            cosobj->type = COS_TYPE_INT;
            cosobj->u.i = result;

            *cosobj_out = cosobj;

            *offset_out = offset;

            return 0;
        }
        num[len] = c - '0';
        offset++;
    }
    return -1; /* number too long */
}

int cos_decode_string(struct pdf_doc *doc,
                      uint64_t *offset_out,
                      struct cos_object **cosobj_out)
{
    return -1;
}

int cos_decode_hex_string(struct pdf_doc *doc,
                      uint64_t *offset_out,
                      struct cos_object **cosobj_out)
{
    return -1;
}


int cos_decode_dictionary(struct pdf_doc *doc,
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

    printf("found a dictionary\n");

    cosobj = calloc(1, sizeof(struct cos_object));
    if (cosobj == NULL) {
        return -1; /* memory error */
    }
    cosobj->type = COS_TYPE_DICTIONARY;

    while ((DOC_BYTE(doc, offset) != '>') &&
           (DOC_BYTE(doc, offset + 1) != '>')) {

        res = cos_decode_object(doc, &offset, &key);
        if (res != 0) {
            /* todo free up any dictionary entries already created */
            printf("key object decode failed\n");
            return res;
        }
        if (key->type != COS_TYPE_NAME) {
            /* key value pairs without a name */
            printf("key was %d not a name %d\n", key->type, COS_TYPE_NAME);
            return -1; /* syntax error */
        }
        printf("key: %s\n", key->u.n);

        res = cos_decode_object(doc, &offset, &value);
        if (res != 0) {
            /* todo free up any dictionary entries already created */
            return res;
        }

        /* add dictionary entry */
        entry = calloc(1, sizeof(struct cos_dictionary_entry));
        if (entry == NULL) {
            /* todo free up any dictionary entries already created */
            return -1; /* memory error */
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

    return 0;
}

int cos_decode_list(struct pdf_doc *doc,
                      uint64_t *offset_out,
                      struct cos_object **cosobj_out)
{
    return -1;
}

#define NAME_MAX_LENGTH 127

/**
 * decode a name object
 *
 * \todo deal with # symbols on pdf versions 1.2 and later
 */
int cos_decode_name(struct pdf_doc *doc,
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
    printf("found a name\n");

    c = DOC_BYTE(doc, offset++);
    while ((idx <= NAME_MAX_LENGTH) &&
           ((bclass[c] & (BC_WSPC | BC_DELM)) == 0)) {
        //printf("%c", c);
        name[idx++] = c;
        c = DOC_BYTE(doc, offset++);
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
        return -1; /* memory error */
    }

    cosobj->type = COS_TYPE_NAME;
    cosobj->u.n = strdup(name);

    *cosobj_out = cosobj;

    *offset_out = offset;

    return 0;
}


int cos_decode_boolean(struct pdf_doc *doc,
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
        return -1; /* memory error */
    }

    cosobj->type = COS_TYPE_BOOL;
    cosobj->u.b = value;

    *cosobj_out = cosobj;

    *offset_out = offset;

    return 0;

}

int cos_decode_null(struct pdf_doc *doc,
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
        return -1; /* memory error */
    }

    cosobj->type = COS_TYPE_NULL;
    *offset_out = offset;

    return 0;
}

/**
 * attempt to decode the stream into a reference
 *
 * The stream has already had a positive integer decoded from it. if another
 * positive integer follows and a R character after that it is a reference,
 * otherwise bail, but not finding a ref is not an error!
 *
 * \param doc the pdf document
 * \param offset_out offset of current cursor in stream
 * \param cosobj_out the object to return into, on input contains the first
 * integer
 */
int cos_attempt_decode_reference(struct pdf_doc *doc,
                      uint64_t *offset_out,
                      struct cos_object **cosobj_out)
{
    uint64_t offset;
    struct cos_object *cosobj; /* possible generation object */
    uint8_t c;
    int res;
    struct cos_reference *nref; /* new reference */

    offset = *offset_out;

    res = cos_decode_object(doc, &offset, &cosobj);
    if (res != 0) {
        return 0; /* no error if object could not be decoded */
    }

    if (cosobj->type != COS_TYPE_INT) {
        /* next object was not an integer so not a reference */
        cos_free_object(cosobj);
        return 0;
    }

    if (cosobj->u.i < 0) {
        /* integer was negative so not a reference (generations must be
         * non-negative
         */
        cos_free_object(cosobj);
        return 0;

    }

    /* two int in a row, look for the R */
    c = DOC_BYTE(doc, offset++);
    if (c != 'R') {
        /* no R so not a reference */
        cos_free_object(cosobj);
        return 0;
    }

    /* found reference */

    printf("found reference\n");
    doc_skip_ws(doc, &offset);

    nref = calloc(1, sizeof(struct cos_reference));
    if (nref == NULL) {
        /* todo free objects */
        return -1; /* memory error */
    }

    nref->id = (*cosobj_out)->u.i;
    nref->generation = cosobj->u.i;

    cos_free_object(*cosobj_out);

    cosobj->type = COS_TYPE_REFERENCE;
    cosobj->u.reference = nref;

    *cosobj_out = cosobj;

    *offset_out = offset;

    return 0;
}

/**
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
 *   object_reference;
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
 */
int cos_decode_object(struct pdf_doc *doc,
                      uint64_t *offset_out,
                      struct cos_object **cosobj_out)
{
    uint64_t offset;
    int res;
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
        res = -1; /* syntax error */
    }


    if (res == 0) {
        *cosobj_out = cosobj;
        *offset_out = offset;
    }

    return res;
}



int decode_trailer(struct pdf_doc *doc, uint64_t offset)
{
    struct cos_object *trailer;
    int res;

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

    return 0;
}

int decode_xref(struct pdf_doc *doc, uint64_t *offset_out)
{
    int res;
    uint64_t objnum; /* current object number */
    uint64_t lastobjnum;
    uint64_t offset;

    offset = *offset_out;

    /* xref object header */
    if ((DOC_BYTE(doc, offset    ) != 'x') &&
        (DOC_BYTE(doc, offset + 1) != 'r') &&
        (DOC_BYTE(doc, offset + 2) != 'e') &&
        (DOC_BYTE(doc, offset + 3) != 'f')) {
        return -1;
    }
    offset += 4;
    doc_skip_ws(doc, &offset);

    /* first object number in table */
    res = doc_read_uint(doc, &offset, &objnum);
    while (res == 0) {
        doc_skip_ws(doc, &offset);

        /* last object number in table */
        res = doc_read_uint(doc, &offset, &lastobjnum);
        if (res != 0) {
            return res;
        }
        doc_skip_ws(doc, &offset);

        lastobjnum += objnum;

        /* object index entries */
        while (objnum < lastobjnum) {
            uint64_t obj_start;
            uint64_t obj_generation;

            /* object offset */
            res = doc_read_uint(doc, &offset, &obj_start);
            if (res != 0) {
                return res;
            }
            doc_skip_ws(doc, &offset);

            res = doc_read_uint(doc, &offset, &obj_generation);
            if (res != 0) {
                return res;
            }
            doc_skip_ws(doc, &offset);

            if ((DOC_BYTE(doc, offset) == 'n')) {
                cos_indirect_object_add(doc, objnum, obj_start, obj_generation);
            }
            offset++;
            doc_skip_ws(doc, &offset);

            objnum++;
        }
        //        printf("at objnum %"PRIu64"\n", objnum);

        /* first object number in table */
        res = doc_read_uint(doc, &offset, &objnum);
    }
    *offset_out = offset;
    return 0;
}

int main(int argc, char **argv)
{
    struct pdf_doc doc;
    int res;
    uint64_t startxref;

    res = read_whole_pdf(&doc, argv[1]);
    if (res != 0) {
        printf("failed to read file\n");
        return res;
    }

    res = check_header(&doc);
    if (res != 0) {
        printf("header check failed\n");
        return res;
    }

    res = find_startxref(&doc, &startxref);
    if (res != 0) {
        printf("failed to find startxref\n");
        return res;
    }

    res = decode_xref(&doc, &startxref);
    if (res != 0) {
        printf("failed to decode xref table\n");
        return res;
    }

    res = decode_trailer(&doc, startxref);
    if (res != 0) {
        printf("failed to decode trailer\n");
        return res;
    }

    return 0;
}
