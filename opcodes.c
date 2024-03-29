#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <dirent.h>

#include "novapy.h"
#include "prints.h"
#include "execute.h"

void POP_TOP(Context *c, uint16_t arg) {
    context_stack_pop(c);
}

void BINARY_MULTIPLY(Context *c, uint16_t arg) {
    PyObject *lhs, *rhs;
    rhs = context_stack_pop(c);
    lhs = context_stack_pop(c);
    switch(arg) {
        case 0: // <
            if(lhs->type == TYPE_INT) {
                int lhs_val = *(int *) lhs->data;
                int rhs_val = *(int *) rhs->data;

                PyObject *obj = malloc(sizeof(PyObject));
                obj->type = TYPE_INT;
                obj->data = malloc(4);
                *(int *) obj->data = lhs_val * rhs_val;
                context_stack_push(c, obj);
                return;
            }
            break;
    }

    fprintf(stderr, "BINARY_MULTIPLY not implemented\n");
    context_stack_push(c, pyc_gen_none());
}

void BINARY_DIVIDE(Context *c, uint16_t arg) {
    PyObject *lhs, *rhs;
    rhs = context_stack_pop(c);
    lhs = context_stack_pop(c);
    switch(arg) {
        case 0: // <
            if(lhs->type == TYPE_INT) {
                int lhs_val = *(int *) lhs->data;
                int rhs_val = *(int *) rhs->data;

                PyObject *obj = malloc(sizeof(PyObject));
                obj->type = TYPE_INT;
                obj->data = malloc(4);
                *(int *) obj->data = lhs_val / rhs_val;
                context_stack_push(c, obj);
                return;
            }
            break;
    }

    fprintf(stderr, "BINARY_MULTIPLY not implemented\n");
    context_stack_push(c, pyc_gen_none());
}

void BINARY_ADD(Context *c, uint16_t arg) {
    PyObject *lhs, *rhs;
    rhs = context_stack_pop(c);
    lhs = context_stack_pop(c);
    switch(arg) {
        case 0: // <
            if(lhs->type == TYPE_INT) {
                int lhs_val = *(int *) lhs->data;
                int rhs_val = *(int *) rhs->data;

                PyObject *obj = malloc(sizeof(PyObject));
                obj->type = TYPE_INT;
                obj->data = malloc(4);
                *(int *) obj->data = lhs_val + rhs_val;
                context_stack_push(c, obj);
                return;
            }
            break;
    }

    fprintf(stderr, "BINARY_MULTIPLY not implemented\n");
    context_stack_push(c, pyc_gen_none());
}

void PRINT_ITEM(Context *c, uint16_t arg) {
    /* CodeObject *co = c->obj->data; */
    /* struct pyc_tuple *tuple = co->consts->data; */
    /* printf("%s", (char *) tuple->items[arg]->data); */
    PyObject *obj = context_stack_pop(c);
    switch(obj->type) {
        case TYPE_STRING:
        case TYPE_INTERNED:
            printf("%s", (char *) obj->data);
            break;
        case TYPE_INT:
            printf("%u", *(int *) obj->data);
            break;
        case TYPE_NONE:
            printf("None");
            break;
        case TYPE_CODE_OBJECT:
            printf("<co:%p>", ((CodeObject *) obj->data)->name);
            break;
        default:
            fprintf(stderr, "Invalid type for PRINT_ITEM: %02hhX\n", obj->type);
            break;
    }
}

void PRINT_NEWLINE(Context *c, uint16_t arg) {
    puts("");
}

void RETURN_VALUE(Context *c, uint16_t arg) {
    c->ret = context_stack_pop(c);
}

void STORE_NAME(Context *c, uint16_t arg) {
    PyObject *obj = context_stack_pop(c);
    CodeObject *co = c->obj->data;
    co->names_value[arg] = *obj;
}

void LOAD_CONST(Context *c, uint16_t arg) {
    CodeObject *co = c->obj->data;
    struct pyc_tuple *tuple = co->consts->data;
    context_stack_push(c, tuple->items[arg]);
    /* fprintf(stderr, "\nLOAD_CONST: "); */
    /* print_pyc_py_object(context_stack_top(c)); */
}

void LOAD_NAME(Context *c, uint16_t arg) {
    CodeObject *co = c->obj->data;
    context_stack_push(c, &co->names_value[arg]);

/*     fprintf(stderr, "\nLOAD_NAME: "); */
/*     print_pyc_py_object(context_stack_top(c)); */
}

void LOAD_ATTR(Context *c, uint16_t arg) {
    CodeObject *co = c->obj->data;
    PyObject *obj = context_stack_pop(c);
    CodeObject *tos = obj->data;

    struct pyc_tuple *context_names = co->names->data;
    char *context_name = repr(context_names->items[arg]);
    struct pyc_tuple *tos_names = tos->names->data;
    int tos_name_index = -1;

    for(int i = 0; i < tos_names->count; i++) {
        if(strcmp(context_name, repr(tos_names->items[i])) == 0) {
            tos_name_index = i;
            break;
        }
    }

    if(tos_name_index == -1) {
        fprintf(stderr, "Invalid attr\n");
        return;
    }

    PyObject *attr = &tos->names_value[tos_name_index];
    context_stack_push(c, attr);
}

void COMPARE_OP(Context *c, uint16_t arg) {
    /* ('<', '<=', '==', '!=', '>', '>=', 'in', 'not in', 'is', 'is not', 'exception match', 'BAD') */
    PyObject *lhs, *rhs;
    rhs = context_stack_pop(c);
    lhs = context_stack_pop(c);
    bool result;
    bool error = false;
    switch(arg) {
        case 0: // <
            if(lhs->type == TYPE_INT && rhs->type == TYPE_INT) {
                int lhs_val = *(int *) lhs->data;
                int rhs_val = *(int *) rhs->data;
                result = lhs_val < rhs_val;
            }
            else {
                error = true;
            }
            break;
        case 2: // ==
            if(lhs->type == TYPE_INT && rhs->type == TYPE_INT) {
                int lhs_val = *(int *) lhs->data;
                int rhs_val = *(int *) rhs->data;
                result = lhs_val == rhs_val;
            }
            else if(lhs->type == TYPE_NONE && rhs->type == TYPE_INTERNED) {
                result = rhs->data == NULL || strlen(rhs->data) == 0;
            }
            else if(lhs->type == TYPE_INTERNED && rhs->type == TYPE_INTERNED) {
                result = strcmp(lhs->data, rhs->data) == 0;
            }
            else {
                error = true;
            }
            break;
    }

    if(error) {
        fprintf(stderr, "COMPARE_OP %d not implemented for type 0x%02X op 0x%02X\n", arg, lhs->type, rhs->type);
        context_stack_push(c, pyc_gen_bool(false));
    }
    else {
        context_stack_push(c, pyc_gen_bool(result));
    }
}


void IMPORT_NAME(Context *c, uint16_t arg) {
    int path_count = 1;
    char *path[] = {
        "./lib/"
    };

    CodeObject *co = c->obj->data;
    struct pyc_tuple *names = co->names->data;
    char *name_orig = names->items[arg]->data;
    char *name = malloc(strlen(name_orig) + 5);
    strcpy(name, name_orig);
    strcat(name, ".pyc");

    char *import_path = NULL;
    for(int i = 0; i < path_count; i++) {
        bool found = false;
        char *p = path[i];
        struct dirent *dp;
        DIR *dfd = opendir(p);
        if(dfd != NULL) {
            while((dp = readdir(dfd)) != NULL) {
                if(strcmp(name, dp->d_name) == 0) {
                    found = true;
                    break;
                }
            }
            closedir(dfd);
        }

        if(found) {
            import_path = malloc(strlen(name) + strlen(p) + 2);
            strcpy(import_path, p);
            strcat(import_path, name);
            fprintf(stderr, "\nImport path: '%s'", import_path);
            break;
        }
    }

    if(import_path == NULL) {
        // This should raise ImportError
        fprintf(stderr, "!! ImportError: %s\n", name);
    }
    else {
        FILE *fh = fopen(import_path, "rb");
        fseek(fh, sizeof(struct pyc_header), SEEK_SET);
        PyObject *module = pyc_read_object(fh, NULL);
        fclose(fh);
        pyc_execute(module);
        context_stack_push(c, module);
    }
}

void POP_JUMP_IF_FALSE(Context *c, uint16_t arg) {
    if(context_stack_pop(c)->type == TYPE_FALSE) {
        c->ip = arg;
    }
}

void CALL_FUNCTION(Context *c, uint16_t arg) {
    /* CodeObject *co = c->obj->data; */
    PyObject **args = calloc(arg, sizeof(PyObject *));

    for(int i = 0; i < arg; i++) {
        args[i] = context_stack_pop(c);
    }

    /* struct pyc_tuple *t = co->names->data; */
    /* char *name = (char *) t->items[arg]->data; */
    PyObject *function = context_stack_pop(c);
    /* fprintf(stderr, "Type: %X\n", function->type); */

    if(function->type == TYPE_INTERNAL_NATIVEFUNCTION) {
        fprintf(stderr, "   <nfunction:%s>\n", (char *) function->data);
        /* fprintf(stderr, "Native function!\n"); */
        if(strcmp(function->data, "input") == 0) {
            printf((char *) args[0]->data);
            char *str = malloc(128);
            scanf("%s", str);
            context_stack_push(c, pyc_gen_string(str));
        }
        else if(strcmp(function->data, "int") == 0) {
            /* PyObject *obj = context_stack_pop(c); */
            if(args[0]->type == TYPE_STRING) {
                context_stack_push(c, pyc_gen_int(atoi(args[0]->data)));
            }
            else {
                fprintf(stderr, "INVALID INT PARAMETER\n");
                context_stack_push(c, pyc_gen_int(0));
            }
        }
    }
    else if(function->type == TYPE_CODE_OBJECT) {
        context_stack_push(c, pyc_execute(function));
    }
    else {
        fprintf(stderr, "\ninvalid function type! 0x%X", function->type);
    }
}

void MAKE_FUNCTION(Context *c, uint16_t arg) {
    PyObject *tos = context_stack_pop(c);
    /* print_pyc_py_object(tos); */
    context_stack_push(c, tos);
}

void (*opa[256])(Context *, uint16_t) = {
    [0 ... 255] = NULL,

    /* [0] = &STOP_CODE, */
    [1] = &POP_TOP,
    /* [2] = &ROT_TWO, */
    /* [3] = &ROT_THREE, */
    /* [4] = &DUP_TOP, */
    /* [5] = &ROT_FOUR, */
    /* [9] = &NOP, */
    /* [10] = &UNARY_POSITIVE, */
    /* [11] = &UNARY_NEGATIVE, */
    /* [12] = &UNARY_NOT, */
    /* [13] = &UNARY_CONVERT, */
    /* [15] = &UNARY_INVERT, */
    /* [19] = &BINARY_POWER, */
    [20] = &BINARY_MULTIPLY,
    [21] = &BINARY_DIVIDE,
    /* [22] = &BINARY_MODULO, */
    [23] = &BINARY_ADD,
    /* [24] = &BINARY_SUBTRACT, */
    /* [25] = &BINARY_SUBSCR, */
    /* [26] = &BINARY_FLOOR_DIVIDE, */
    /* [27] = &BINARY_TRUE_DIVIDE, */
    /* [28] = &INPLACE_FLOOR_DIVIDE, */
    /* [29] = &INPLACE_TRUE_DIVIDE, */
    /* [30] = &SLICE_0, */
    /* [31] = &SLICE_1, */
    /* [32] = &SLICE_2, */
    /* [33] = &SLICE_3, */
    /* [40] = &STORE_SLICE_0, */
    /* [41] = &STORE_SLICE_1, */
    /* [42] = &STORE_SLICE_2, */
    /* [43] = &STORE_SLICE_3, */
    /* [50] = &DELETE_SLICE_0, */
    /* [51] = &DELETE_SLICE_1, */
    /* [52] = &DELETE_SLICE_2, */
    /* [53] = &DELETE_SLICE_3, */
    /* [54] = &STORE_MAP, */
    /* [55] = &INPLACE_ADD, */
    /* [56] = &INPLACE_SUBTRACT, */
    /* [57] = &INPLACE_MULTIPLY, */
    /* [58] = &INPLACE_DIVIDE, */
    /* [59] = &INPLACE_MODULO, */
    /* [60] = &STORE_SUBSCR, */
    /* [61] = &DELETE_SUBSCR, */
    /* [62] = &BINARY_LSHIFT, */
    /* [63] = &BINARY_RSHIFT, */
    /* [64] = &BINARY_AND, */
    /* [65] = &BINARY_XOR, */
    /* [66] = &BINARY_OR, */
    /* [67] = &INPLACE_POWER, */
    /* [68] = &GET_ITER, */
    /* [70] = &PRINT_EXPR, */
    [71] = &PRINT_ITEM,
    [72] = &PRINT_NEWLINE,
    /* [73] = &PRINT_ITEM_TO, */
    /* [74] = &PRINT_NEWLINE_TO, */
    /* [75] = &INPLACE_LSHIFT, */
    /* [76] = &INPLACE_RSHIFT, */
    /* [77] = &INPLACE_AND, */
    /* [78] = &INPLACE_XOR, */
    /* [79] = &INPLACE_OR, */
    /* [80] = &BREAK_LOOP, */
    /* [81] = &WITH_CLEANUP, */
    /* [82] = &LOAD_LOCALS, */
    [83] = &RETURN_VALUE,
    /* [84] = &IMPORT_STAR, */
    /* [85] = &EXEC_STMT, */
    /* [86] = &YIELD_VALUE, */
    /* [87] = &POP_BLOCK, */
    /* [88] = &END_FINALLY, */
    /* [89] = &BUILD_CLASS, */
    [90] = &STORE_NAME,
    /* [91] = &DELETE_NAME, */
    /* [92] = &UNPACK_SEQUENCE, */
    /* [93] = &FOR_ITER, */
    /* [94] = &LIST_APPEND, */
    /* [95] = &STORE_ATTR, */
    /* [96] = &DELETE_ATTR, */
    /* [97] = &STORE_GLOBAL, */
    /* [98] = &DELETE_GLOBAL, */
    /* [99] = &DUP_TOPX, */
    [100] = &LOAD_CONST,
    [101] = &LOAD_NAME,
    /* [102] = &BUILD_TUPLE, */
    /* [103] = &BUILD_LIST, */
    /* [104] = &BUILD_SET, */
    /* [105] = &BUILD_MAP, */
    [106] = &LOAD_ATTR,
    [107] = &COMPARE_OP,
    [108] = &IMPORT_NAME,
    /* [109] = &IMPORT_FROM, */
    /* [110] = &JUMP_FORWARD, */
    /* [111] = &JUMP_IF_FALSE_OR_POP, */
    /* [112] = &JUMP_IF_TRUE_OR_POP, */
    /* [113] = &JUMP_ABSOLUTE, */
    [114] = &POP_JUMP_IF_FALSE,
    /* [115] = &POP_JUMP_IF_TRUE, */
    /* [116] = &LOAD_GLOBAL, */
    /* [119] = &CONTINUE_LOOP, */
    /* [120] = &SETUP_LOOP, */
    /* [121] = &SETUP_EXCEPT, */
    /* [122] = &SETUP_FINALLY, */
    /* [124] = &LOAD_FAST, */
    /* [125] = &STORE_FAST, */
    /* [126] = &DELETE_FAST, */
    /* [130] = &RAISE_VARARGS, */
    [131] = &CALL_FUNCTION,
    [132] = &MAKE_FUNCTION,
    /* [133] = &BUILD_SLICE, */
    /* [134] = &MAKE_CLOSURE, */
    /* [135] = &LOAD_CLOSURE, */
    /* [136] = &LOAD_DEREF, */
    /* [137] = &STORE_DEREF, */
    /* [140] = &CALL_FUNCTION_VAR, */
    /* [141] = &CALL_FUNCTION_KW, */
    /* [142] = &CALL_FUNCTION_VAR_KW, */
    /* [143] = &SETUP_WITH, */
    /* [145] = &EXTENDED_ARG, */
    /* [146] = &SET_ADD, */
    /* [147] = &MAP_ADD, */
};

