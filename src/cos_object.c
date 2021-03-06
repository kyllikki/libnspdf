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
#include "cos_content.h"
#include "cos_object.h"
#include "cos_parse.h"
#include "pdf_doc.h"

static nspdferror cos_dump_object(const char *fmt, struct cos_object *cos_obj)
{
    printf("%s\n", fmt);
    switch (cos_obj->type) {
    case COS_TYPE_NAMETREE:
        printf("  type = COS_TYPE_NAMETREE\n");
        break;

    case COS_TYPE_NUMBERTREE:
        printf("  type = COS_TYPE_NUMBERTREE\n");
        break;

    case COS_TYPE_REFERENCE:
        printf("  type = COS_TYPE_REFERENCE\n"
               "  u.reference->id = %lu\n"
               "  u.reference->generation = %lu\n",
               cos_obj->u.reference->id,
               cos_obj->u.reference->generation);
        break;

    case COS_TYPE_NULL:
        printf("  type = COS_TYPE_NULL\n");
        break;

    case COS_TYPE_CONTENT:
        printf("  type = COS_TYPE_CONTENT\n"
               "  u.content->length = %d\n"
               "  u.content->alloc = %d\n"
               "  u.content->operations = %p\n",
               cos_obj->u.content->length,
               cos_obj->u.content->alloc,
               cos_obj->u.content->operations);
        break;

    case COS_TYPE_BOOL:
        printf("  type = COS_TYPE_BOOL\n  u.b = %s\n",
               cos_obj->u.b ? "true" : "false");
        break;

    case COS_TYPE_INT:
        printf("  type = COS_TYPE_INT\n  u.i = %ld\n", cos_obj->u.i);
        break;

    case COS_TYPE_REAL:
        printf("  type = COS_TYPE_REAL\n  u.real = %f\n", cos_obj->u.real);
        break;

    case COS_TYPE_NAME:
        printf("  type = COS_TYPE_NAME\n  u.name = %s\n", cos_obj->u.name);
        break;

    case COS_TYPE_STRING:
        printf("  type = COS_TYPE_STRING\n");
        //free(cos_obj->u.s->data);
        //free(cos_obj->u.s);
        break;

    case COS_TYPE_DICTIONARY:
        printf("  type = COS_TYPE_DICTIONARY\n");
        /*
          dentry = cos_obj->u.dictionary;
          while (dentry != NULL) {
          struct cos_dictionary_entry *odentry;

          cos_free_object(dentry->key);
          cos_free_object(dentry->value);

          odentry = dentry;
          dentry = dentry->next;
          free(odentry);
          }
        */
        break;

    case COS_TYPE_ARRAY:
        printf("  type = COS_TYPE_ARRAY\n");
        /*
          if (cos_obj->u.array->alloc > 0) {
          for (aentry = 0; aentry < cos_obj->u.array->length; aentry++) {
          cos_free_object(*(cos_obj->u.array->values + aentry));
          }
          free(cos_obj->u.array->values);
          }
          free(cos_obj->u.array);
        */
        break;

    case COS_TYPE_STREAM:
        printf("  type = COS_TYPE_STREAM\n");
        /*
          free(cos_obj->u.stream);
        */
        break;

    }


    return NSPDFERROR_OK;
}

nspdferror cos_free_object(struct cos_object *cos_obj)
{
    struct cos_dictionary_entry *dentry;
    unsigned int aentry;

    switch (cos_obj->type) {
    case COS_TYPE_NAME:
        free(cos_obj->u.name);
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
cos_extract_dictionary_value(struct nspdf_doc *doc,
                             struct cos_object *dict,
                             const char *key,
                             struct cos_object **value_out)
{
    nspdferror res;

    res = nspdf__xref_get_referenced(doc, &dict);
    if (res == NSPDFERROR_OK) {
        if (dict->type != COS_TYPE_DICTIONARY) {
            res = NSPDFERROR_TYPE;

        } else {
            struct cos_dictionary_entry **prev;
            struct cos_dictionary_entry *entry;

            res = NSPDFERROR_NOTFOUND;

            prev = &dict->u.dictionary;
            entry = *prev;
            while (entry != NULL) {
                if (strcmp(entry->key->u.name, key) == 0) {
                    *value_out = entry->value;
                    *prev = entry->next;
                    cos_free_object(entry->key);
                    free(entry);
                    res = NSPDFERROR_OK;
                    break;
                }
                prev = &entry->next;
                entry = *prev;
            }
        }
    }
    return res;

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
                if (strcmp(entry->key->u.name, key) == 0) {
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


/* get an inheritable array object from a dictionary */
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
cos_get_number(struct nspdf_doc *doc,
               struct cos_object *cobj,
               float *value_out)
{
    nspdferror res;

    res = nspdf__xref_get_referenced(doc, &cobj);
    if (res == NSPDFERROR_OK) {
        if (cobj->type == COS_TYPE_INT) {
            *value_out = (float)cobj->u.i;
        } else if (cobj->type == COS_TYPE_REAL) {
            *value_out = cobj->u.real;
        } else {
            res = NSPDFERROR_TYPE;
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
            *value_out = cobj->u.name;
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
cos_get_rectangle(struct nspdf_doc *doc,
                  struct cos_object *cobj,
                  struct cos_rectangle *rect_out)
{
    nspdferror res;
    struct cos_rectangle rect;

    res = nspdf__xref_get_referenced(doc, &cobj);
    if (res == NSPDFERROR_OK) {
        if ((cobj->type != COS_TYPE_ARRAY) ||
            (cobj->u.array->length != 4)) {
            res = NSPDFERROR_TYPE;
        } else {
            res = cos_get_number(doc, cobj->u.array->values[0], &rect.llx);
            if (res == NSPDFERROR_OK) {
                res = cos_get_number(doc, cobj->u.array->values[1], &rect.lly);
                if (res == NSPDFERROR_OK) {
                    res = cos_get_number(doc, cobj->u.array->values[2], &rect.urx);
                    if (res == NSPDFERROR_OK) {
                        res = cos_get_number(doc, cobj->u.array->values[3], &rect.ury);
                        if (res == NSPDFERROR_OK) {
                            *rect_out = rect;
                        }
                    }
                }
            }
        }
    }

    return res;
}


/*
 * exported interface documented in cos_object.h
 *
 * slightly different behaviour to other getters:
 *
 * - This getter can be passed an object pointer to a synthetic parsed content
 *     stream object in which case it returns that objects content operation
 *     list.
 *
 * - Alternatively it can be passed a single indirect object reference to a
 *    content stream which will be processed into a filtered stream and then
 *    converted into a parsed content stream which replaces the passed
 *    object. The underlying filtered streams will then be freed.
 *
 * - An array of indirect object references to content streams all of which
 *    will be converted as if a single stream of tokens and the result handled
 *    as per the single reference case.
 */
nspdferror
cos_get_content(struct nspdf_doc *doc,
                struct cos_object *cobj,
                struct cos_content **content_out)
{
    nspdferror res;
    struct cos_object **references;
    unsigned int reference_count;
    struct cos_stream **streams;
    unsigned int index;
    struct cos_object *content_obj; /* parsed content object */
    struct cos_object tmpobj;

    //cos_dump_object("get content of", cobj);

    if (cobj->type == COS_TYPE_CONTENT) {
        /* already parsed the content stream */
        goto cos_get_content_done;
    }

    if (cobj->type == COS_TYPE_REFERENCE) {
        /* single reference */
        reference_count = 1;
        references = calloc(reference_count, sizeof(struct cos_object *));
        if (references == NULL) {
            return NSPDFERROR_NOMEM;
        }

        *references = cobj;
    } else if (cobj->type == COS_TYPE_ARRAY) {
        /* array of references */
        reference_count = cobj->u.array->length;
        references = malloc(reference_count * sizeof(struct cos_object *));
        if (references == NULL) {
            return NSPDFERROR_NOMEM;
        }
        memcpy(references, cobj->u.array->values, reference_count * sizeof(struct cos_object *));
        /* check all objects in array are references */
        for (index = 0; index < reference_count ; index++) {
            if ((*(references + index))->type != COS_TYPE_REFERENCE) {
                free(references);
                return NSPDFERROR_TYPE;
            }
        }
    } else {
        return NSPDFERROR_TYPE;
    }

    /* obtain array of streams */
    streams = malloc(reference_count * sizeof(struct cos_stream *));
    if (streams == NULL) {
        free(references);
        return NSPDFERROR_TYPE;
    }

    for (index = 0; index < reference_count ; index++) {
        struct cos_object *stream_obj;

        stream_obj = *(references + index);
        res = nspdf__xref_get_referenced(doc, &stream_obj);
        if (res != NSPDFERROR_OK) {
            free(references);
            free(streams);
            return res;
        }
        if (stream_obj->type != COS_TYPE_STREAM) {
            free(references);
            free(streams);
            return NSPDFERROR_TYPE;
        }
        *(streams + index) = stream_obj->u.stream;;
    }

    res = cos_parse_content_streams(doc, streams, reference_count, &content_obj);
    if (res != NSPDFERROR_OK) {
        free(references);
        free(streams);
        return res;
    }

    /* replace passed object with parsed content operations object */
    tmpobj = *cobj;
    *cobj = *content_obj;
    *content_obj = tmpobj;

    //cos_dump_object("content object", cobj);
    //cos_dump_object("free object", content_obj);

    cos_free_object(content_obj);

    /** \todo call nspdf__xref_free_referenced(doc, *(references + index)); to free up storage associated with already parsed streams */

cos_get_content_done:
    *content_out = cobj->u.content;

    return NSPDFERROR_OK;
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
