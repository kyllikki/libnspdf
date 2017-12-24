struct pdf_doc;

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

struct cos_array_entry {
    /** next value in array */
    struct cos_array_entry *next;

    /** value */
    struct cos_object *value;
};

struct cos_string {
    uint8_t *data;
    size_t length;
    size_t alloc;
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
        struct cos_string *s;

        /** stream data */
        uint8_t *stream;

        /* dictionary */
        struct cos_dictionary_entry *dictionary;

        /* array */
        struct cos_array_entry *array;

        /** reference */
        struct cos_reference *reference;

    } u;
};

/**
 * Decode input stream into an object
 *
 * lex and parse a byte stream to generate a COS object.
 */
nspdferror cos_decode_object(struct pdf_doc *doc, uint64_t *offset_out, struct cos_object **cosobj_out);

nspdferror cos_free_object(struct cos_object *cos_obj);

nspdferror cos_dictionary_get_value(struct cos_object *dict, const char *key, struct cos_object **value_out);

nspdferror cos_dictionary_extract_value(struct cos_object *dict, const char *key, struct cos_object **value_out);


nspdferror cos_get_int(struct pdf_doc *doc, struct cos_object *cobj, int64_t *value_out);

nspdferror cos_get_dictionary(struct pdf_doc *doc, struct cos_object *cobj, struct cos_object **value_out);


