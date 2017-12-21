#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "nspdferror.h"
#include "cos_object.h"


nspdferror cos_free_object(struct cos_object *cos_obj)
{
    struct cos_dictionary_entry *dentry;
    struct cos_array_entry *aentry;

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
        aentry = cos_obj->u.array;
        while (aentry != NULL) {
            struct cos_array_entry *oaentry;

            cos_free_object(aentry->value);

            oaentry = aentry;
            aentry = aentry->next;
            free(oaentry);
        }

    case COS_TYPE_STREAM:
        free(cos_obj->u.stream);
        break;

    }
    free(cos_obj);

    return NSPDFERROR_OK;
}

nspdferror
cos_dictionary_get_value(struct cos_object *dict,
                         const char *key,
                         struct cos_object **value_out)
{
    struct cos_dictionary_entry *entry;

    if (dict->type != COS_TYPE_DICTIONARY) {
        return NSPDFERROR_TYPE;
    }

    entry = dict->u.dictionary;
    while (entry != NULL) {
        if (strcmp(entry->key->u.n, key) == 0) {
            *value_out = entry->value;
            return NSPDFERROR_OK;
        }
        entry = entry->next;
    }
    return NSPDFERROR_NOTFOUND;
}

/**
 * extracts a value for a key in a dictionary.
 *
 * this finds and returns a value for a given key removing it from a dictionary
 */
nspdferror
cos_dictionary_extract_value(struct cos_object *dict,
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

nspdferror cos_get_int(struct cos_object *cobj, int64_t *value_out)
{
    if (cobj->type != COS_TYPE_INT) {
        return NSPDFERROR_TYPE;
    }
    *value_out = cobj->u.i;
    return NSPDFERROR_OK;
}

nspdferror
cos_get_dictionary(struct cos_object *cobj,
                   struct cos_object **value_out)
{
    if (cobj->type == COS_TYPE_REFERENCE) {
        
    }
    if (cobj->type != COS_TYPE_DICTIONARY) {
        return NSPDFERROR_TYPE;
    }
    *value_out = cobj;
    return NSPDFERROR_OK;
}
