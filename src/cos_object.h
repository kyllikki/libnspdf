struct nspdf_doc;

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

nspdferror cos_free_object(struct cos_object *cos_obj);

/**
 * extract a value for a key from a dictionary
 *
 * This retrieves the value of a given key in a dictionary and removes it from
 * the dictionary.
 *
 * \param dict The dictionary
 * \param key The key to lookup
 * \param value_out The value object associated with the key
 * \return NSPDFERROR_OK and value_out updated on success.
 *         NSPDFERROR_TYPE if the object passed in \p dict is not a dictionary.
 *         NSPDFERROR_NOTFOUND if the key is not present in the dictionary.
 */
nspdferror cos_extract_dictionary_value(struct cos_object *dict, const char *key, struct cos_object **value_out);

/**
 * get a value for a key from a dictionary
 *
 * \param dict The dictionary
 * \param key The key to lookup
 * \param value_out The value object associated with the key
 * \return NSPDFERROR_OK and value_out updated on success.
 *         NSPDFERROR_TYPE if the object passed in \p dict is not a dictionary.
 *         NSPDFERROR_NOTFOUND if the key is not present in the dictionary.
 */
nspdferror cos_get_dictionary_value(struct nspdf_doc *doc, struct cos_object *dict, const char *key, struct cos_object **value_out);


nspdferror cos_get_dictionary_int(struct nspdf_doc *doc, struct cos_object *dict, const char *key, int64_t *value_out);


nspdferror cos_get_dictionary_name(struct nspdf_doc *doc, struct cos_object *dict, const char *key, const char **value_out);

nspdferror cos_get_dictionary_dictionary(struct nspdf_doc *doc, struct cos_object *dict, const char *key, struct cos_object **value_out);

nspdferror cos_heritable_dictionary_dictionary(struct nspdf_doc *doc, struct cos_object *dict, const char *key, struct cos_object **value_out);

nspdferror cos_get_dictionary_array(struct nspdf_doc *doc, struct cos_object *dict, const char *key, struct cos_object **value_out);

nspdferror cos_heritable_dictionary_array(struct nspdf_doc *doc, struct cos_object *dict, const char *key, struct cos_object **value_out);


nspdferror cos_get_int(struct nspdf_doc *doc, struct cos_object *cobj, int64_t *value_out);


nspdferror cos_get_name(struct nspdf_doc *doc, struct cos_object *cobj, const char **value_out);


nspdferror cos_get_dictionary(struct nspdf_doc *doc, struct cos_object *cobj, struct cos_object **value_out);

nspdferror cos_get_array(struct nspdf_doc *doc, struct cos_object *cobj, struct cos_object **value_out);

nspdferror cos_get_array_size(struct nspdf_doc *doc, struct cos_object *cobj, unsigned int *size_out);

nspdferror cos_get_array_value(struct nspdf_doc *doc, struct cos_object *array, unsigned int index, struct cos_object **value_out);

nspdferror cos_get_array_dictionary(struct nspdf_doc *doc, struct cos_object *arrau, unsigned int index, struct cos_object **value_out);
