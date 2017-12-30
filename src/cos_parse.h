struct nspdf_doc;
struct cos_object;

/**
 * Decode input stream into an object
 *
 * lex and parse a byte stream to generate a COS object.
 */
nspdferror cos_parse_object(struct nspdf_doc *doc, uint64_t *offset_out, struct cos_object **cosobj_out);

