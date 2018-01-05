/** indirect object */
struct xref_table_entry {
    /* reference identifier */
    struct cos_reference ref;

    /** offset of object */
    uint64_t offset;

    /* indirect object if already decoded */
    struct cos_object *object;
};

/** page entry */
struct page_table_entry {
    struct cos_object *resources;
    struct cos_object *mediabox;
    struct cos_object *contents;
};

/** pdf document */
struct nspdf_doc {

    const uint8_t *start; /* start of pdf document in input stream */
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

    /* page refrerence table */
    uint64_t page_table_size;
    struct page_table_entry *page_table;
};

/* byte data acessory, allows for more complex buffer handling in future */
#define DOC_BYTE(doc, offset) (doc->start[(offset)])

nspdferror doc_skip_ws(struct nspdf_doc *doc, uint64_t *offset);
nspdferror doc_skip_eol(struct nspdf_doc *doc, uint64_t *offset);

nspdferror xref_get_referenced(struct nspdf_doc *doc, struct cos_object **cobj_out);

nspdferror nspdf__decode_page_tree(struct nspdf_doc *doc, struct cos_object *page_tree_node, unsigned int *page_index);
