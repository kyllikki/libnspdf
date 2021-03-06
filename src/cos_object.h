/*
 * Copyright 2018 Vincent Sanders <vince@netsurf-browser.org>
 *
 * This file is part of libnspdf.
 *
 * Licensed under the MIT License,
 *                http://www.opensource.org/licenses/mit-license.php
 */

/**
 * \file
 * NetSurf PDF library COS objects
 */

#ifndef NSPDF__COS_OBJECT_H_
#define NSPDF__COS_OBJECT_H_

#include "cos_stream.h"

struct nspdf_doc;
struct content_operation;
struct cos_content;
struct cos_object;

/**
 * The type of cos object in an entry.
 */
enum cos_type {
    COS_TYPE_NULL, /* 0 - NULL object */
    COS_TYPE_BOOL,
    COS_TYPE_INT,
    COS_TYPE_REAL,
    COS_TYPE_NAME,
    COS_TYPE_STRING,
    COS_TYPE_ARRAY, /* 6 */
    COS_TYPE_DICTIONARY,
    COS_TYPE_NAMETREE,
    COS_TYPE_NUMBERTREE,
    COS_TYPE_STREAM,
    COS_TYPE_REFERENCE, /* 11 */
    COS_TYPE_CONTENT, /* 12 - parsed content stream */
};


/**
 * list of COS dictionary entries.
 */
struct cos_dictionary_entry {
    /** next key/value in dictionary */
    struct cos_dictionary_entry *next;

    /** key (name) */
    struct cos_object *key;

    /** value */
    struct cos_object *value;
};


/**
 * array of COS objects
 */
struct cos_array {
    /** number of values */
    unsigned int length;

    /** number of allocated values */
    unsigned int alloc;

    /** array of object pointers */
    struct cos_object **values;
};


/**
 * COS string data
 */
struct cos_string {
    unsigned int length; /**< string length */
    size_t alloc; /**< memory allocation for string */
    uint8_t *data; /**< string data */
};


/**
 * reference to COS object
 */
struct cos_reference {
    uint64_t id; /**< id of indirect object */
    uint64_t generation; /**< generation of indirect object */
};


/**
 * COS rectangle
 */
struct cos_rectangle {
    float llx; /**< lower left x */
    float lly; /**< lower left y */
    float urx; /**< upper right x */
    float ury; /**< upper right y */
};

/**
 * Carosel object
 */
struct cos_object {
    enum cos_type type;
    union {
        /** boolean */
        bool b;

        /** integer */
        int64_t i;

        /** real */
        float real;

        /** name */
        char *name;

        /** string */
        struct cos_string *s;

        /** stream data */
        struct cos_stream *stream;

        /** dictionary */
        struct cos_dictionary_entry *dictionary;

        /** array */
        struct cos_array *array;

        /** reference */
        struct cos_reference *reference;

        /** parsed content stream */
        struct cos_content *content;
    } u;
};

nspdferror cos_free_object(struct cos_object *cos_obj);


/**
 * extract a value object for a key from a dictionary
 *
 * This retrieves the value object of a given key in a dictionary and removes
 * the entry from the dictionary. Once extracted the caller owns the returned
 * object and must free it.
 *
 * Get the value for a key from a dictionary, If the dictionary is an object
 * reference it will be dereferenced first which will parse any previously
 * unreferenced indirect objects.
 *
 * \param doc The document the cos object belongs to or NULL to supress dereferencing.
 * \param dict The dictionary
 * \param key The key to lookup
 * \param value_out The value object associated with the key
 * \return NSPDFERROR_OK and value_out updated on success.
 *         NSPDFERROR_TYPE if the object passed in \p dict is not a dictionary.
 *         NSPDFERROR_NOTFOUND if the key is not present in the dictionary.
 */
nspdferror cos_extract_dictionary_value(struct nspdf_doc *doc, struct cos_object *dict, const char *key, struct cos_object **value_out);


/**
 * get a value object for a key from a dictionary
 *
 * Get the value for a key from a dictionary, If the dictionary is an object
 * reference it will be dereferenced first which will parse any
 * previously unreferenced indirect objects.
 *
 * \param doc The document the cos object belongs to or NULL to supress dereferencing.
 * \param dict The dictionary
 * \param key The key to lookup
 * \param value_out The value object associated with the key
 * \return NSPDFERROR_OK and value_out updated on success.
 *         NSPDFERROR_TYPE if the object passed in \p dict is not a dictionary.
 *         NSPDFERROR_NOTFOUND if the key is not present in the dictionary.
 */
nspdferror cos_get_dictionary_value(struct nspdf_doc *doc, struct cos_object *dict, const char *key, struct cos_object **value_out);


/**
 * get an integer value for a key from a dictionary
 *
 * Get the integer value for a key from a dictionary, If the dictionary is an
 * object reference it will be dereferenced first which will parse any
 * previously unreferenced indirect objects.
 *
 * \param doc The document the cos object belongs to or NULL to supress dereferencing.
 * \param dict The dictionary
 * \param key The key to lookup
 * \param in_out The integer value associated with the key.
 * \return NSPDFERROR_OK and value_out updated on success.
 *         NSPDFERROR_TYPE if the object passed in \p dict is not a dictionary
 *           or the value of the key is not an integer.
 *         NSPDFERROR_NOTFOUND if the key is not present in the dictionary.
 */
nspdferror cos_get_dictionary_int(struct nspdf_doc *doc, struct cos_object *dict, const char *key, int64_t *int_out);


nspdferror cos_get_dictionary_name(struct nspdf_doc *doc, struct cos_object *dict, const char *key, const char **value_out);


nspdferror cos_get_dictionary_string(struct nspdf_doc *doc, struct cos_object *dict, const char *key, struct cos_string **string_out);


nspdferror cos_get_dictionary_dictionary(struct nspdf_doc *doc, struct cos_object *dict, const char *key, struct cos_object **value_out);


nspdferror cos_heritable_dictionary_dictionary(struct nspdf_doc *doc, struct cos_object *dict, const char *key, struct cos_object **value_out);


nspdferror cos_get_dictionary_array(struct nspdf_doc *doc, struct cos_object *dict, const char *key, struct cos_object **value_out);


nspdferror cos_heritable_dictionary_array(struct nspdf_doc *doc, struct cos_object *dict, const char *key, struct cos_object **value_out);


nspdferror cos_get_array_size(struct nspdf_doc *doc, struct cos_object *cobj, unsigned int *size_out);


nspdferror cos_get_array_value(struct nspdf_doc *doc, struct cos_object *array, unsigned int index, struct cos_object **value_out);


nspdferror cos_get_array_dictionary(struct nspdf_doc *doc, struct cos_object *arrau, unsigned int index, struct cos_object **value_out);

/**
 * get the integer value of a cos object.
 *
 * Get the value from a cos object, if the object is an object reference it
 *  will be dereferenced first. The dereferencing will parse any previously
 *  unreferenced indirect objects as required.
 *
 * \param doc The document the cos object belongs to.
 * \param cobj A cos object of integer type.
 * \param value_out The result value.
 * \return NSERROR_OK and \p value_out updated,
 *         NSERROR_TYPE if the \p cobj is not an integer
 */
nspdferror cos_get_int(struct nspdf_doc *doc, struct cos_object *cobj, int64_t *value_out);


/**
 * get the float value of a cos object.
 *
 * Get the value from a cos object, if the object is an object reference it
 *  will be dereferenced first. The dereferencing will parse any previously
 *  unreferenced indirect objects as required.
 *
 * \param doc The document the cos object belongs to.
 * \param cobj A cos object of integer type.
 * \param value_out The result value.
 * \return NSERROR_OK and \p value_out updated,
 *         NSERROR_TYPE if the \p cobj is not an integer
 */
nspdferror cos_get_number(struct nspdf_doc *doc, struct cos_object *cobj, float *value_out);


/**
 * get the name value of a cos object.
 *
 * Get the value from a cos object, if the object is an object reference it
 *  will be dereferenced first. The dereferencing will parse any previously
 *  unreferenced indirect objects as required.
 *
 * \param doc The document the cos object belongs to.
 * \param cobj A cos object of name type.
 * \param name_out The result value.
 * \return NSERROR_OK and \p value_out updated,
 *         NSERROR_TYPE if the \p cobj is not a name
 */
nspdferror cos_get_name(struct nspdf_doc *doc, struct cos_object *cobj, const char **name_out);


/**
 * get the string value of a cos object.
 *
 * Get the value from a cos object, if the object is an object reference it
 *  will be dereferenced first. The dereferencing will parse any previously
 *  unreferenced indirect objects as required.
 *
 * \param doc The document the cos object belongs to.
 * \param cobj A cos object of string type.
 * \param string_out The result value.
 * \return NSERROR_OK and \p value_out updated,
 *         NSERROR_TYPE if the \p cobj is not a string
 */
nspdferror cos_get_string(struct nspdf_doc *doc, struct cos_object *cobj, struct cos_string **string_out);


/**
 * get the dictionary value of a cos object.
 *
 * Get the value from a cos object, if the object is an object reference it
 *  will be dereferenced first. The dereferencing will parse any previously
 *  unreferenced indirect objects as required.
 *
 * \param doc The document the cos object belongs to.
 * \param cobj A cos object of dictionary type.
 * \param value_out The result value.
 * \return NSERROR_OK and \p value_out updated,
 *         NSERROR_TYPE if the \p cobj is not a dictionary
 */
nspdferror cos_get_dictionary(struct nspdf_doc *doc, struct cos_object *cobj, struct cos_object **value_out);


/**
 * get the array value of a cos object.
 *
 * Get the value from a cos object, if the object is an object reference it
 *  will be dereferenced first. The dereferencing will parse any previously
 *  unreferenced indirect objects as required.
 *
 * \param doc The document the cos object belongs to.
 * \param cobj A cos object of array type.
 * \param value_out The result value.
 * \return NSERROR_OK and \p value_out updated,
 *         NSERROR_TYPE if the \p cobj is not a array
 */
nspdferror cos_get_array(struct nspdf_doc *doc, struct cos_object *cobj, struct cos_object **value_out);


/**
 * get the stream value of a cos object.
 *
 * Get the value from a cos object, if the object is an object reference it
 *  will be dereferenced first. The dereferencing will parse any previously
 *  unreferenced indirect objects as required.
 *
 * \param doc The document the cos object belongs to.
 * \param cobj A cos object of stream type.
 * \param stream_out The result value.
 * \return NSERROR_OK and \p stream_out updated,
 *         NSERROR_TYPE if the \p cobj is not a array
 */
nspdferror cos_get_stream(struct nspdf_doc *doc, struct cos_object *cobj, struct cos_stream **stream_out);


/**
 * get a direct cos object.
 *
 * Obtain a direct object if the passed object was a reference it is
 * dereferenced from the cross reference table.
 *
 * \param doc The document the cos object belongs to.
 * \param cobj A cos object.
 * \param object_out The result object.
 * \return NSERROR_OK and \p object_out updated,
 */
nspdferror cos_get_object(struct nspdf_doc *doc, struct cos_object *cobj, struct cos_object **object_out);


/**
 * get a parsed content object
 *
 * Get the parsed content from a cos object, if the object is an object
 *   reference it will be dereferenced first.
 * The parsed content object is *not* a normal COS object rather it is the
 *   internal result of parsing a PDF content stream.
 * This object type is used to replace the stream object in the cross reference
 *   table after its initial parse to avoid the need to keep and repeatedly
 *   parse the filtered stream data.
 *
 */
nspdferror cos_get_content(struct nspdf_doc *doc, struct cos_object *cobj, struct cos_content **content_out);


/**
 * Get a rectangle
 *
 * Generates a synthetic rectangle object from a array of four numbers
 *
 * \param doc The document the cos object belongs to.
 * \param cobj A cos object.
 * \param rect_out The result rectangle.
 * \return NSERROR_OK and \p rect_out updated,
 */
nspdferror cos_get_rectangle(struct nspdf_doc *doc, struct cos_object *cobj, struct cos_rectangle *rect_out);

#endif
