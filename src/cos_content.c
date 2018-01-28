/*
 * Copyright 2018 Vincent Sanders <vince@netsurf-browser.org>
 *
 * This file is part of libnspdf.
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

#include "cos_object.h"
#include "cos_content.h"
#include "pdf_doc.h"

static const char*operator_name(enum content_operator operator)
{
    switch(operator) {
    case CONTENT_OP_b: return "b";
    case CONTENT_OP_B: return "B";
    case CONTENT_OP_b_: return "b*";
    case CONTENT_OP_B_: return "B*";
    case CONTENT_OP_BDC: return "BDC";
    case CONTENT_OP_BI: return "BI";
    case CONTENT_OP_BMC: return "BMC";
    case CONTENT_OP_BT: return "BT";
    case CONTENT_OP_BX: return "BX";
    case CONTENT_OP_c: return "c";
    case CONTENT_OP_cm: return "cm";
    case CONTENT_OP_CS: return "CS";
    case CONTENT_OP_cs: return "cs";
    case CONTENT_OP_d: return "d";
    case CONTENT_OP_d0: return "d0";
    case CONTENT_OP_d1: return "d1";
    case CONTENT_OP_Do: return "Do";
    case CONTENT_OP_DP: return "DP";
    case CONTENT_OP_EI: return "EI";
    case CONTENT_OP_EMC: return "EMC";
    case CONTENT_OP_ET: return "ET";
    case CONTENT_OP_EX: return "EX";
    case CONTENT_OP_f: return "f";
    case CONTENT_OP_F: return "F";
    case CONTENT_OP_f_: return "f*";
    case CONTENT_OP_G: return "G";
    case CONTENT_OP_g: return "g";
    case CONTENT_OP_gs: return "gs";
    case CONTENT_OP_h: return "h";
    case CONTENT_OP_i: return "i";
    case CONTENT_OP_ID: return "ID";
    case CONTENT_OP_j: return "j";
    case CONTENT_OP_J: return "J";
    case CONTENT_OP_K: return "K";
    case CONTENT_OP_k: return "k";
    case CONTENT_OP_l: return "l";
    case CONTENT_OP_m: return "m";
    case CONTENT_OP_M: return "M";
    case CONTENT_OP_MP: return "MP";
    case CONTENT_OP_n: return "n";
    case CONTENT_OP_q: return "q";
    case CONTENT_OP_Q: return "Q";
    case CONTENT_OP_re: return "re";
    case CONTENT_OP_RG: return "RG";
    case CONTENT_OP_rg: return "rg";
    case CONTENT_OP_ri: return "ri";
    case CONTENT_OP_s: return "s";
    case CONTENT_OP_S: return "S";
    case CONTENT_OP_SC: return "SC";
    case CONTENT_OP_sc: return "sc";
    case CONTENT_OP_SCN: return "SCN";
    case CONTENT_OP_scn: return "scn";
    case CONTENT_OP_sh: return "sh";
    case CONTENT_OP_T_: return "T*";
    case CONTENT_OP_Tc: return "Tc";
    case CONTENT_OP_Td: return "Td";
    case CONTENT_OP_TD: return "TD";
    case CONTENT_OP_Tf: return "Tf";
    case CONTENT_OP_Tj: return "Tj";
    case CONTENT_OP_TJ: return "TJ";
    case CONTENT_OP_TL: return "TL";
    case CONTENT_OP_Tm: return "Tm";
    case CONTENT_OP_Tr: return "Tr";
    case CONTENT_OP_Ts: return "Ts";
    case CONTENT_OP_Tw: return "Tw";
    case CONTENT_OP_Tz: return "Tz";
    case CONTENT_OP_v: return "v";
    case CONTENT_OP_w: return "w";
    case CONTENT_OP_W: return "W";
    case CONTENT_OP_W_: return "W_";
    case CONTENT_OP_y: return "y";
    case CONTENT_OP__: return "\'";
    case CONTENT_OP___: return "\"";
    }
    return "????";
}


/**
 * move number operands from list into operation
 *
 * This ensures all operands are correctly handled not just the wanted ones
 *
 * \param wanted The number of wanted operands to place in the operation
 * \param operands The array of operands from the parse
 * \param operand_idx The number of operands from the parse
 * \param operation_out The operation to place numbers in
 */
static nspdferror
copy_numbers(unsigned int wanted,
             struct cos_object **operands,
             unsigned int *operand_idx,
             struct content_operation *operation_out)
{
    nspdferror res;
    unsigned int index = 0;

    while ((index < (*operand_idx)) &&
           (index < wanted)) {
        /* process wanted operands */
        res = cos_get_number(NULL,
                             *(operands + index),
                             &operation_out->u.number[index]);
        if (res != NSPDFERROR_OK) {
            printf("operand %d could not be set in operation (code %d)\n",
                   index, res);
        }
        cos_free_object(*(operands + index));
        index++;
    }
    if ((*operand_idx) > index) {
        printf("operator %s that takes %d operands passed %d\n",
               operator_name(operation_out->operator), wanted, *operand_idx);
        while (index < (*operand_idx)) {
            cos_free_object(*(operands + index));
            index++;
        }
    } else if ((*operand_idx) < index) {
        printf("operator %s that takes %d operands passed %d\n",
               operator_name(operation_out->operator), wanted, *operand_idx);
    }

    *operand_idx = 0; /* all operands freed */

    return NSPDFERROR_OK;
}

static nspdferror
copy_integers(unsigned int wanted,
             struct cos_object **operands,
             unsigned int *operand_idx,
             struct content_operation *operation_out)
{
    nspdferror res;
    unsigned int index = 0;

    while ((index < (*operand_idx)) &&
           (index < wanted)) {
        /* process wanted operands */
        res = cos_get_int(NULL,
                             *(operands + index),
                             &operation_out->u.i[index]);
        if (res != NSPDFERROR_OK) {
            printf("operand %d could not be set in operation (code %d)\n",
                   index, res);
        }
        cos_free_object(*(operands + index));
        index++;
    }
    if ((*operand_idx) > index) {
        printf("operator %s that takes %d operands passed %d\n",
               operator_name(operation_out->operator), wanted, *operand_idx);
        while (index < (*operand_idx)) {
            cos_free_object(*(operands + index));
            index++;
        }
    } else if ((*operand_idx) < index) {
        printf("operator %s that takes %d operands passed %d\n",
               operator_name(operation_out->operator), wanted, *operand_idx);
    }

    *operand_idx = 0; /* all operands freed */

    return NSPDFERROR_OK;
}

static nspdferror
copy_string(struct cos_object **operands,
             unsigned int *operand_idx,
             struct content_operation *operation_out)
{
    nspdferror res;
    unsigned int index = 0;
    struct cos_string *string;

    if ((*operand_idx) == 0) {
        printf("operator %s that takes %d operands passed %d\n",
               operator_name(operation_out->operator), 1, *operand_idx);
        operation_out->u.string.length = 0;
        return NSPDFERROR_OK;
    }

    /* process wanted operands */
    res = cos_get_string(NULL, *operands, &string);
    if (res != NSPDFERROR_OK) {
        printf("string could not be set in operation (code %d)\n", res);
        operation_out->u.string.length = 0;
    } else {
        operation_out->u.string.length = string->length;
        if (string->length > content_string_intrnl_lngth) {
            /* steal the string from the object */
            operation_out->u.string.u.pdata = string->data;
            string->alloc = 0;
            string->length = 0;
            /*printf("external string \"%.*s\"\n",
              operation_out->u.string.length,
              operation_out->u.string.u.pdata);*/
        } else {
            memcpy(operation_out->u.string.u.cdata,
                   string->data,
                   string->length);
            /*printf("internal string \"%.*s\"\n",
              operation_out->u.string.length,
              operation_out->u.string.u.cdata);*/
        }
    }

    if ((*operand_idx) > 1) {
        printf("operator %s that takes %d operands passed %d\n",
               operator_name(operation_out->operator), 1, *operand_idx);
    }

    /* free all operands */
    while (index < (*operand_idx)) {
        cos_free_object(*(operands + index));
        index++;
    }
    *operand_idx = 0;

    return NSPDFERROR_OK;
}

static nspdferror
copy_array(struct cos_object **operands,
             unsigned int *operand_idx,
             struct content_operation *operation_out)
{
    unsigned int index = 0;

    if ((*operand_idx) == 0) {
        printf("operator %s that takes %d operands passed %d\n",
               operator_name(operation_out->operator), 1, *operand_idx);
        operation_out->u.array.length = 0;
        return NSPDFERROR_OK;
    }

    /* process wanted operands */
    if ((*operands)->type != COS_TYPE_ARRAY) {
        printf("operand was not an array\n");
        operation_out->u.array.length = 0;
    } else {
        operation_out->u.array.length = (*operands)->u.array->length;
        /* steal the values from the array object */
        operation_out->u.array.values = (*operands)->u.array->values;
        (*operands)->u.array->alloc = 0;
        (*operands)->u.array->length = 0;
    }

    if ((*operand_idx) > 1) {
        printf("operator %s that takes %d operands passed %d\n",
               operator_name(operation_out->operator), 1, *operand_idx);
    }

    /* free all operands */
    while (index < (*operand_idx)) {
        cos_free_object(*(operands + index));
        index++;
    }
    *operand_idx = 0;

    return NSPDFERROR_OK;
}


static nspdferror
copy_name(struct cos_object **operands,
          unsigned int *operand_idx,
          struct content_operation *operation_out)
{
    unsigned int index = 0;

    if ((*operand_idx) == 0) {
        printf("operator %s that takes %d operands passed %d\n",
               operator_name(operation_out->operator), 1, *operand_idx);
        operation_out->u.name = NULL;
        return NSPDFERROR_OK;
    }

    /* process wanted operands */
    if ((*operands)->type != COS_TYPE_NAME) {
        printf("operand was not a name\n");
        operation_out->u.name = NULL;
    } else {
        /* steal the name from the name object */
        operation_out->u.name = (*operands)->u.name;
        (*operands)->u.name = NULL;
    }

    if ((*operand_idx) > 1) {
        printf("operator %s that takes %d operands passed %d\n",
               operator_name(operation_out->operator), 1, *operand_idx);
    }

    /* free all operands */
    while (index < (*operand_idx)) {
        cos_free_object(*(operands + index));
        index++;
    }
    *operand_idx = 0;

    return NSPDFERROR_OK;
}

static nspdferror
copy_name_number(struct cos_object **operands,
             unsigned int *operand_idx,
             struct content_operation *operation_out)
{
    unsigned int index = 0;

    if ((*operand_idx) == 0) {
        printf("operator %s that takes %d operands passed %d\n",
               operator_name(operation_out->operator), 2, *operand_idx);
        operation_out->u.namenumber.name = NULL;
        return NSPDFERROR_OK;
    }

    /* process wanted operands */
    if ((*operands)->type != COS_TYPE_NAME) {
        printf("operand was not a name\n");
        operation_out->u.namenumber.name = NULL;
    } else {
        /* steal the name from the name object */
        operation_out->u.namenumber.name = (*operands)->u.name;
        (*operands)->u.name = NULL;

        operation_out->u.namenumber.number = 0;
        /* get the number */
        if ((*operand_idx) > 1) {
            nspdferror res;
            res = cos_get_number(NULL,
                                 *(operands + 1),
                                 &operation_out->u.namenumber.number);
            if (res != NSPDFERROR_OK) {
                printf("operand 1 could not be set in operation (code %d)\n", res);
            }
        } else {
            printf("operator %s that takes %d operands passed %d\n",
                   operator_name(operation_out->operator), 2, *operand_idx);
        }
    }

    if ((*operand_idx) > 2) {
        printf("operator %s that takes %d operands passed %d\n",
               operator_name(operation_out->operator), 2, *operand_idx);
    }

    /* free all operands */
    while (index < (*operand_idx)) {
        cos_free_object(*(operands + index));
        index++;
    }
    *operand_idx = 0;

    return NSPDFERROR_OK;
}


static nspdferror
copy_array_int(struct cos_object **operands,
             unsigned int *operand_idx,
             struct content_operation *operation_out)
{
    unsigned int index = 0;

    if ((*operand_idx) == 0) {
        printf("operator %s that takes %d operands passed %d\n",
               operator_name(operation_out->operator), 2, *operand_idx);
        operation_out->u.namenumber.name = NULL;
        return NSPDFERROR_OK;
    }

    /* process wanted operands */
    if ((*operands)->type != COS_TYPE_ARRAY) {
        printf("operand was not an array\n");
        operation_out->u.arrayint.length = 0;
        operation_out->u.arrayint.values = NULL;
        operation_out->u.arrayint.i = 0;
    } else {
        operation_out->u.arrayint.length = (*operands)->u.array->length;
        /* steal the values from the array object */
        operation_out->u.arrayint.values = (*operands)->u.array->values;
        (*operands)->u.array->alloc = 0;
        (*operands)->u.array->length = 0;

        operation_out->u.arrayint.i = 0;
        /* get the int */
        if ((*operand_idx) > 1) {
            nspdferror res;
            res = cos_get_int(NULL, *(operands + 1), &operation_out->u.arrayint.i);
            if (res != NSPDFERROR_OK) {
                printf("operand 1 could not be set in operation (code %d)\n", res);
            }
        } else {
            printf("operator %s that takes %d operands passed %d\n",
                   operator_name(operation_out->operator), 2, *operand_idx);
        }
    }

    if ((*operand_idx) > 2) {
        printf("operator %s that takes %d operands passed %d\n",
               operator_name(operation_out->operator), 2, *operand_idx);
    }

    /* free all operands */
    while (index < (*operand_idx)) {
        cos_free_object(*(operands + index));
        index++;
    }
    *operand_idx = 0;

    return NSPDFERROR_OK;
}

/* exported interface documented in cos_content.h */
nspdferror
nspdf__cos_content_convert(enum content_operator operator,
                           struct cos_object **operands,
                           unsigned int *operand_idx,
                           struct content_operation *operation_out)
{
    nspdferror res;

    operation_out->operator = operator;

    switch (operator) {
    case CONTENT_OP_b:
    case CONTENT_OP_B:
    case CONTENT_OP_b_:
    case CONTENT_OP_B_:
    case CONTENT_OP_BI:
    case CONTENT_OP_BT:
    case CONTENT_OP_BX:
    case CONTENT_OP_EI:
    case CONTENT_OP_EMC:
    case CONTENT_OP_ET:
    case CONTENT_OP_EX:
    case CONTENT_OP_f:
    case CONTENT_OP_F:
    case CONTENT_OP_f_:
    case CONTENT_OP_h:
    case CONTENT_OP_ID:
    case CONTENT_OP_n:
    case CONTENT_OP_q:
    case CONTENT_OP_Q:
    case CONTENT_OP_s:
    case CONTENT_OP_S:
    case CONTENT_OP_T_:
    case CONTENT_OP_W:
    case CONTENT_OP_W_:
        /* no operands */
        res = copy_numbers(0, operands, operand_idx, operation_out);
        break;

    case CONTENT_OP_G:
    case CONTENT_OP_g:
    case CONTENT_OP_i:
    case CONTENT_OP_M:
    case CONTENT_OP_Tc:
    case CONTENT_OP_TL:
    case CONTENT_OP_Ts:
    case CONTENT_OP_Tw:
    case CONTENT_OP_Tz:
    case CONTENT_OP_w:
        /* one number */
        res = copy_numbers(1, operands, operand_idx, operation_out);
        break;

    case CONTENT_OP_d0:
    case CONTENT_OP_l:
    case CONTENT_OP_m:
    case CONTENT_OP_Td:
    case CONTENT_OP_TD:
        /* two numbers */
        res = copy_numbers(2, operands, operand_idx, operation_out);
        break;

    case CONTENT_OP_RG:
    case CONTENT_OP_rg:
        /* three numbers */
        res = copy_numbers(3, operands, operand_idx, operation_out);
        break;

    case CONTENT_OP_K:
    case CONTENT_OP_k:
    case CONTENT_OP_re:
    case CONTENT_OP_v:
    case CONTENT_OP_y:
        /* four numbers */
        res = copy_numbers(4, operands, operand_idx, operation_out);
        break;

    case CONTENT_OP_c:
    case CONTENT_OP_cm:
    case CONTENT_OP_d1:
    case CONTENT_OP_Tm:
        /* six numbers */
        res = copy_numbers(6, operands, operand_idx, operation_out);
        break;

    case CONTENT_OP_Tj:
    case CONTENT_OP__:
        /* single string */
        res = copy_string(operands, operand_idx, operation_out);
        break;

    case CONTENT_OP_TJ:
        /* single array */
        res = copy_array(operands, operand_idx, operation_out);
        break;

    case CONTENT_OP_Tf:
        /* name and number */
        res = copy_name_number(operands, operand_idx, operation_out);
        break;

    case CONTENT_OP_gs:
    case CONTENT_OP_Do:
    case CONTENT_OP_ri:
    case CONTENT_OP_CS:
    case CONTENT_OP_cs:
    case CONTENT_OP_sh:
    case CONTENT_OP_MP:
    case CONTENT_OP_BMC:
        /* name */
        res = copy_name(operands, operand_idx, operation_out);
        break;

    case CONTENT_OP_j:
    case CONTENT_OP_J:
    case CONTENT_OP_Tr:
        /* one integer */
        res = copy_integers(1, operands, operand_idx, operation_out);
        break;

    case CONTENT_OP_d:
        /* array and int */
        res = copy_array_int(operands, operand_idx, operation_out);
        break;

    case CONTENT_OP_BDC:
    case CONTENT_OP_DP:
    case CONTENT_OP_SC:
    case CONTENT_OP_sc:
    case CONTENT_OP_SCN:
    case CONTENT_OP_scn:
    case CONTENT_OP___:
        res = copy_numbers(0, operands, operand_idx, operation_out);
        break;
    }

    return res;
}
