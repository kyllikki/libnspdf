/** indirect object */
struct xref_table_entry {
    /* reference identifier */
    struct cos_reference ref;

    /** offset of object */
    uint64_t offset;

    /* indirect object if already decoded */
    struct cos_object *object;
};


/** pdf document */
struct pdf_doc {
    uint8_t *buffer;
    uint64_t buffer_length;

    uint8_t *start; /* start of pdf document in input stream */
    uint64_t length;

    int major;
    int minor;

    /**
     * Indirect object cross reference table
     */
    uint64_t xref_size;
    struct xref_table_entry *xref_table;

    struct cos_object *root;
    struct cos_object *encrypt;
    struct cos_object *info;
    struct cos_object *id;

};

/* byte data acessory, allows for more complex buffer handling in future */
#define DOC_BYTE(doc, offset) (doc->start[(offset)])

nspdferror doc_skip_ws(struct pdf_doc *doc, uint64_t *offset);
nspdferror doc_skip_eol(struct pdf_doc *doc, uint64_t *offset);

nspdferror xref_get_referenced(struct pdf_doc *doc, struct cos_object **cobj_out);
