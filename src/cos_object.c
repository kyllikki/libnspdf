/*
 * Copyright 2018 Vincent Sanders <vince@netsurf-browser.org>
 *
 * This file is part of libnspsl
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

#include "xref.h"
#include "cos_object.h"
#include "cos_parse.h"
#include "pdf_doc.h"


nspdferror cos_free_object(struct cos_object *cos_obj)
{
    struct cos_dictionary_entry *dentry;
    unsigned int aentry;

    switch (cos_obj->type) {
    case COS_TYPE_NAME:
        free(cos_obj->u.n);
        break;

    case COS_TYPE_STRING:
        free(cos_obj->u.s->data);
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

    case COS_TYPE_ARRAY:
        if (cos_obj->u.array->alloc > 0) {
            for (aentry = 0; aentry < cos_obj->u.array->length; aentry++) {
                cos_free_object(*(cos_obj->u.array->values + aentry));
            }
            free(cos_obj->u.array->values);
        }
        free(cos_obj->u.array);
        break;

    case COS_TYPE_STREAM:
        free(cos_obj->u.stream);
        break;

    }
    free(cos_obj);

    return NSPDFERROR_OK;
}


/*
 * extracts a value for a key in a dictionary.
 *
 * this finds and returns a value for a given key removing it from a dictionary
 */
nspdferror
cos_extract_dictionary_value(struct cos_object *dict,
                             const char *key,
                             struct cos_object **value_out)
{
    struct cos_dictionary_entry *entry;
    struct cos_dictionary_entry **prev;

    if (dict->type != COS_TYPE_DICTIONARY) {
        return NSPDFERROR_TYPE;
    }

    prev = &dict->u.dictionary;
    entry = *prev;
    while (entry != NULL) {
        if (strcmp(entry->key->u.n, key) == 0) {
            *value_out = entry->value;
            *prev = entry->next;
            cos_free_object(entry->key);
            free(entry);
            return NSPDFERROR_OK;
        }
        prev = &entry->next;
        entry = *prev;
    }
    return NSPDFERROR_NOTFOUND;
}


/*
 * get a value for a key from a dictionary
 */
nspdferror
cos_get_dictionary_value(struct nspdf_doc *doc,
                         struct cos_object *dict,
                         const char *key,
                         struct cos_object **value_out)
{
    nspdferror res;
    struct cos_dictionary_entry *entry;

    res = nspdf__xref_get_referenced(doc, &dict);
    if (res == NSPDFERROR_OK) {
        if (dict->type != COS_TYPE_DICTIONARY) {
            res = NSPDFERROR_TYPE;
        } else {
            res = NSPDFERROR_NOTFOUND;

            entry = dict->u.dictionary;
            while (entry != NULL) {
                if (strcmp(entry->key->u.n, key) == 0) {
                    *value_out = entry->value;
                    res = NSPDFERROR_OK;
                    break;
                }
                entry = entry->next;
            }
        }
    }
    return res;
}

nspdferror
cos_get_dictionary_int(struct nspdf_doc *doc,
                       struct cos_object *dict,
                       const char *key,
                       int64_t *value_out)
{
    nspdferror res;
    struct cos_object *dict_value;

    res = cos_get_dictionary_value(doc, dict, key, &dict_value);
    if (res != NSPDFERROR_OK) {
        return res;
    }
    return cos_get_int(doc, dict_value, value_out);
}

nspdferror
cos_get_dictionary_name(struct nspdf_doc *doc,
                        struct cos_object *dict,
                        const char *key,
                        const char **value_out)
{
    nspdferror res;
    struct cos_object *dict_value;

    res = cos_get_dictionary_value(doc, dict, key, &dict_value);
    if (res != NSPDFERROR_OK) {
        return res;
    }
    return cos_get_name(doc, dict_value, value_out);
}

nspdferror
cos_get_dictionary_string(struct nspdf_doc *doc,
                        struct cos_object *dict,
                        const char *key,
                        struct cos_string **string_out)
{
    nspdferror res;
    struct cos_object *dict_value;

    res = cos_get_dictionary_value(doc, dict, key, &dict_value);
    if (res != NSPDFERROR_OK) {
        return res;
    }
    return cos_get_string(doc, dict_value, string_out);
}

nspdferror
cos_get_dictionary_dictionary(struct nspdf_doc *doc,
                        struct cos_object *dict,
                        const char *key,
                        struct cos_object **value_out)
{
    nspdferror res;
    struct cos_object *dict_value;

    res = cos_get_dictionary_value(doc, dict, key, &dict_value);
    if (res != NSPDFERROR_OK) {
        return res;
    }
    return cos_get_dictionary(doc, dict_value, value_out);
}

nspdferror
cos_heritable_dictionary_dictionary(struct nspdf_doc *doc,
                        struct cos_object *dict,
                        const char *key,
                        struct cos_object **value_out)
{
    nspdferror res;
    struct cos_object *dict_value;
    res = cos_get_dictionary_value(doc, dict, key, &dict_value);
    if (res == NSPDFERROR_NOTFOUND) {
        /* \todo get parent entry and extract key from that dictionary instead */
    }
    if (res != NSPDFERROR_OK) {
        return res;
    }
    return cos_get_dictionary(doc, dict_value, value_out);
}

nspdferror
cos_get_dictionary_array(struct nspdf_doc *doc,
                        struct cos_object *dict,
                        const char *key,
                        struct cos_object **value_out)
{
    nspdferror res;
    struct cos_object *dict_value;

    res = cos_get_dictionary_value(doc, dict, key, &dict_value);
    if (res != NSPDFERROR_OK) {
        return res;
    }
    return cos_get_array(doc, dict_value, value_out);
}

nspdferror
cos_heritable_dictionary_array(struct nspdf_doc *doc,
                        struct cos_object *dict,
                        const char *key,
                        struct cos_object **value_out)
{
    nspdferror res;
    struct cos_object *dict_value;

    res = cos_get_dictionary_value(doc, dict, key, &dict_value);
    if (res == NSPDFERROR_NOTFOUND) {
        /* \todo get parent entry and extract key from that dictionary instead */
    }
    if (res != NSPDFERROR_OK) {
        return res;
    }
    return cos_get_array(doc, dict_value, value_out);
}

nspdferror
cos_get_int(struct nspdf_doc *doc,
            struct cos_object *cobj,
            int64_t *value_out)
{
    nspdferror res;

    res = nspdf__xref_get_referenced(doc, &cobj);
    if (res == NSPDFERROR_OK) {
        if (cobj->type != COS_TYPE_INT) {
            res = NSPDFERROR_TYPE;
        } else {
            *value_out = cobj->u.i;
        }
    }
    return res;
}

nspdferror
cos_get_name(struct nspdf_doc *doc,
            struct cos_object *cobj,
            const char **value_out)
{
    nspdferror res;

    res = nspdf__xref_get_referenced(doc, &cobj);
    if (res == NSPDFERROR_OK) {
        if (cobj->type != COS_TYPE_NAME) {
            res = NSPDFERROR_TYPE;
        } else {
            *value_out = cobj->u.n;
        }
    }
    return res;
}


nspdferror
cos_get_dictionary(struct nspdf_doc *doc,
                   struct cos_object *cobj,
                   struct cos_object **value_out)
{
    nspdferror res;

    res = nspdf__xref_get_referenced(doc, &cobj);
    if (res == NSPDFERROR_OK) {
        if (cobj->type != COS_TYPE_DICTIONARY) {
            res = NSPDFERROR_TYPE;
        } else {
            *value_out = cobj;
        }
    }
    return res;
}


nspdferror
cos_get_array(struct nspdf_doc *doc,
                   struct cos_object *cobj,
                   struct cos_object **value_out)
{
    nspdferror res;

    res = nspdf__xref_get_referenced(doc, &cobj);
    if (res == NSPDFERROR_OK) {
        if (cobj->type != COS_TYPE_ARRAY) {
            res = NSPDFERROR_TYPE;
        } else {
            *value_out = cobj;
        }
    }
    return res;
}


nspdferror
cos_get_string(struct nspdf_doc *doc,
               struct cos_object *cobj,
               struct cos_string **string_out)
{
    nspdferror res;

    res = nspdf__xref_get_referenced(doc, &cobj);
    if (res == NSPDFERROR_OK) {
        if (cobj->type != COS_TYPE_STRING) {
            res = NSPDFERROR_TYPE;
        } else {
            *string_out = cobj->u.s;
        }
    }
    return res;
}


nspdferror
cos_get_stream(struct nspdf_doc *doc,
               struct cos_object *cobj,
               struct cos_stream **stream_out)
{
    nspdferror res;
    //printf("%p %d\n", cobj, cobj->type);
    res = nspdf__xref_get_referenced(doc, &cobj);
    //printf("%p %d res:%d\n", cobj, cobj->type, res);
    if (res == NSPDFERROR_OK) {
        if (cobj->type != COS_TYPE_STREAM) {
            res = NSPDFERROR_TYPE;
        } else {
            *stream_out = cobj->u.stream;
        }
    }
    return res;
}


/*
 * get object from object reference
 */
nspdferror
cos_get_object(struct nspdf_doc *doc,
               struct cos_object *cobj,
               struct cos_object **value_out)
{
    nspdferror res;
    res = nspdf__xref_get_referenced(doc, &cobj);
    if (res == NSPDFERROR_OK) {
        *value_out = cobj;
    }
    return res;
}


nspdferror
cos_get_content(struct nspdf_doc *doc,
                struct cos_object *cobj,
                struct cos_content **content_out)
{
    nspdferror res;
    struct cos_object *content_obj;

    res = nspdf__xref_get_referenced(doc, &cobj);
    if (res == NSPDFERROR_OK) {
        if (cobj->type == COS_TYPE_STREAM) {
            res = cos_parse_content_stream(doc, cobj->u.stream, &content_obj);
            if (res == NSPDFERROR_OK) {
                /* replace stream object with parsed content operations */
                struct cos_object tmpobj;
                tmpobj = *cobj;
                *cobj = *content_obj;
                *content_obj = tmpobj;
                cos_free_object(content_obj);

                *content_out = cobj->u.content;
            }
        } else if (cobj->type == COS_TYPE_CONTENT) {
            *content_out = cobj->u.content;
        } else {
            res = NSPDFERROR_TYPE;
        }
    }
    return res;
}

/*
 * get a value for a key from a dictionary
 */
nspdferror
cos_get_array_value(struct nspdf_doc *doc,
                    struct cos_object *array,
                    unsigned int index,
                    struct cos_object **value_out)
{
    nspdferror res;

    res = nspdf__xref_get_referenced(doc, &array);
    if (res == NSPDFERROR_OK) {
        if (array->type != COS_TYPE_ARRAY) {
            res = NSPDFERROR_TYPE;
        } else {
            if (index >= array->u.array->length) {
                res = NSPDFERROR_RANGE;
            } else {
                *value_out = *(array->u.array->values + index);
            }
        }
    }
    return res;
}

nspdferror
cos_get_array_dictionary(struct nspdf_doc *doc,
                        struct cos_object *array,
                        unsigned int index,
                        struct cos_object **value_out)
{
    nspdferror res;
    struct cos_object *array_value;

    res = cos_get_array_value(doc, array, index, &array_value);
    if (res != NSPDFERROR_OK) {
        return res;
    }
    return cos_get_dictionary(doc, array_value, value_out);
}

nspdferror
cos_get_array_size(struct nspdf_doc *doc,
                   struct cos_object *cobj,
                   unsigned int *size_out)
{
    nspdferror res;

    res = nspdf__xref_get_referenced(doc, &cobj);
    if (res == NSPDFERROR_OK) {
        if (cobj->type != COS_TYPE_ARRAY) {
            res = NSPDFERROR_TYPE;
        } else {
            *size_out = cobj->u.array->length;
        }
    }
    return res;
}
