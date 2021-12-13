#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "novapy.h"
#include "prints.h"
#include "execute.h"

const char *OP_LABELS[150] = {
    [0] = "STOP_CODE",
    [1] = "POP_TOP",
    [2] = "ROT_TWO",
    [3] = "ROT_THREE",
    [4] = "DUP_TOP",
    [5] = "ROT_FOUR",
    [9] = "NOP",
    [10] = "UNARY_POSITIVE",
    [11] = "UNARY_NEGATIVE",
    [12] = "UNARY_NOT",
    [13] = "UNARY_CONVERT",
    [15] = "UNARY_INVERT",
    [19] = "BINARY_POWER",
    [20] = "BINARY_MULTIPLY",
    [21] = "BINARY_DIVIDE",
    [22] = "BINARY_MODULO",
    [23] = "BINARY_ADD",
    [24] = "BINARY_SUBTRACT",
    [25] = "BINARY_SUBSCR",
    [26] = "BINARY_FLOOR_DIVIDE",
    [27] = "BINARY_TRUE_DIVIDE",
    [28] = "INPLACE_FLOOR_DIVIDE",
    [29] = "INPLACE_TRUE_DIVIDE",
    [30] = "SLICE_0",
    [31] = "SLICE_1",
    [32] = "SLICE_2",
    [33] = "SLICE_3",
    [40] = "STORE_SLICE_0",
    [41] = "STORE_SLICE_1",
    [42] = "STORE_SLICE_2",
    [43] = "STORE_SLICE_3",
    [50] = "DELETE_SLICE_0",
    [51] = "DELETE_SLICE_1",
    [52] = "DELETE_SLICE_2",
    [53] = "DELETE_SLICE_3",
    [54] = "STORE_MAP",
    [55] = "INPLACE_ADD",
    [56] = "INPLACE_SUBTRACT",
    [57] = "INPLACE_MULTIPLY",
    [58] = "INPLACE_DIVIDE",
    [59] = "INPLACE_MODULO",
    [60] = "STORE_SUBSCR",
    [61] = "DELETE_SUBSCR",
    [62] = "BINARY_LSHIFT",
    [63] = "BINARY_RSHIFT",
    [64] = "BINARY_AND",
    [65] = "BINARY_XOR",
    [66] = "BINARY_OR",
    [67] = "INPLACE_POWER",
    [68] = "GET_ITER",
    [70] = "PRINT_EXPR",
    [71] = "PRINT_ITEM",
    [72] = "PRINT_NEWLINE",
    [73] = "PRINT_ITEM_TO",
    [74] = "PRINT_NEWLINE_TO",
    [75] = "INPLACE_LSHIFT",
    [76] = "INPLACE_RSHIFT",
    [77] = "INPLACE_AND",
    [78] = "INPLACE_XOR",
    [79] = "INPLACE_OR",
    [80] = "BREAK_LOOP",
    [81] = "WITH_CLEANUP",
    [82] = "LOAD_LOCALS",
    [83] = "RETURN_VALUE",
    [84] = "IMPORT_STAR",
    [85] = "EXEC_STMT",
    [86] = "YIELD_VALUE",
    [87] = "POP_BLOCK",
    [88] = "END_FINALLY",
    [89] = "BUILD_CLASS",
    [90] = "STORE_NAME",
    [91] = "DELETE_NAME",
    [92] = "UNPACK_SEQUENCE",
    [93] = "FOR_ITER",
    [94] = "LIST_APPEND",
    [95] = "STORE_ATTR",
    [96] = "DELETE_ATTR",
    [97] = "STORE_GLOBAL",
    [98] = "DELETE_GLOBAL",
    [99] = "DUP_TOPX",
    [100] = "LOAD_CONST",
    [101] = "LOAD_NAME",
    [102] = "BUILD_TUPLE",
    [103] = "BUILD_LIST",
    [104] = "BUILD_SET",
    [105] = "BUILD_MAP",
    [106] = "LOAD_ATTR",
    [107] = "COMPARE_OP",
    [108] = "IMPORT_NAME",
    [109] = "IMPORT_FROM",
    [110] = "JUMP_FORWARD",
    [111] = "JUMP_IF_FALSE_OR_POP",
    [112] = "JUMP_IF_TRUE_OR_POP",
    [113] = "JUMP_ABSOLUTE",
    [114] = "POP_JUMP_IF_FALSE",
    [115] = "POP_JUMP_IF_TRUE",
    [116] = "LOAD_GLOBAL",
    [119] = "CONTINUE_LOOP",
    [120] = "SETUP_LOOP",
    [121] = "SETUP_EXCEPT",
    [122] = "SETUP_FINALLY",
    [124] = "LOAD_FAST",
    [125] = "STORE_FAST",
    [126] = "DELETE_FAST",
    [130] = "RAISE_VARARGS",
    [131] = "CALL_FUNCTION",
    [132] = "MAKE_FUNCTION",
    [133] = "BUILD_SLICE",
    [134] = "MAKE_CLOSURE",
    [135] = "LOAD_CLOSURE",
    [136] = "LOAD_DEREF",
    [137] = "STORE_DEREF",
    [140] = "CALL_FUNCTION_VAR",
    [141] = "CALL_FUNCTION_KW",
    [142] = "CALL_FUNCTION_VAR_KW",
    [143] = "SETUP_WITH",
    [145] = "EXTENDED_ARG",
    [146] = "SET_ADD",
    [147] = "MAP_ADD",
};

static int OP_ARGC[150] = {
    [0 ... 89] = 0,
    [90 ... 149] = 2,
    /* [0 ... 149] = 2, */
    /* [71] = 0, */
    /* [72] = 0, */
    /* [83] = 0, */
};

static char* TYPE_NAMES[150] = {
    [0x28] = "TUPLE",
    [0x46] = "FALSE",
    [0x4E] = "NONE",
    [0x52] = "STRREF",
    [0x54] = "TRUE",
    [0x63] = "CODEOBJECT",
    [0x69] = "INT",
    [0x73] = "STRING",
    [0x74] = "INTERNED",
    [0x75] = "UNICODE"
};

PyObject *pyc_read_object(FILE *fh, PyObject *root) {
    /* struct pyc_py_object obj; */
    PyObject *obj = malloc(sizeof(PyObject));
    obj->root = root;
    fread(&obj->type, 1, 1, fh);
    fprintf(stderr, "Reading PyObject %s 0x%02hhX\n", TYPE_NAMES[obj->type], obj->type);

    switch(obj->type) {
        case TYPE_CODE_OBJECT: {
            /* printf("Found code object! %lu\n", sizeof(struct pyc_op_arg)); */
            struct pyc_code_object *co = malloc(sizeof(*co));
            obj->data = co;
            co->interned_strings_count = 0;
            co->interned_strings = calloc(16, sizeof(PyObject *));
            /* fread(&co, sizeof(struct pyc_code_object), 1, fh); */
            fread(&co->arg_count, 4, 1, fh);
            fread(&co->local_count, 4, 1, fh);
            fread(&co->stack_size, 4, 1, fh);
            fread(&co->flags, 4, 1, fh);

            // Load Assembly
            co->assembly = malloc(sizeof(Assembly));
            fread(&co->assembly->string_magic, 1, 1, fh);
            fread(&co->assembly->length, 4, 1, fh);
            /* co.length /= 3; */
            print_pyc_code_object(co);

            // Load Assembly
            co->assembly->opargs = calloc(co->assembly->length, sizeof(OpArg));
            /* OpArg *op_arg = co->assembly->opargs; */

            for(unsigned int i = 0; i < co->assembly->length;) {
                OpArg *op_arg = co->assembly->opargs + i;
                /* OpArg *op_arg = malloc(sizeof(*op_arg)); */
                /* co->assembly[co->assembly->op_count] = op_arg; */
                /* printf("Offset: %X\n", ftell(fh)); */
                /* fread(&op_arg, sizeof(struct pyc_op_arg), 1, fh); */
                fread(&op_arg->op_code, 1, 1, fh);

                if(OP_ARGC[op_arg->op_code] > 0) {
                    fread(&op_arg->arg, OP_ARGC[op_arg->op_code], 1, fh);
                }
                /* printf("%-3i %-3d 0x%02X %-20s %04X\n", i, */
                /*     op_arg->op_code, op_arg->op_code, */
                /*     OP_LABELS[op_arg->op_code], op_arg->arg); */
                i += 1 + OP_ARGC[op_arg->op_code];
                /* co->assembly[co->assembly->op_count] = op_arg; */
                op_arg->valid = true;

                /* op_arg++; */
                co->assembly->op_count++;
            }

            // Load consts
            fprintf(stderr, "-- START READING Consts\n");
            co->consts = pyc_read_object(fh, obj);
            fprintf(stderr, "Consts: ");
            print_pyc_py_object(co->consts, true);

            // Load names
            fprintf(stderr, "-- START READING Names\n");
            co->names = pyc_read_object(fh, obj);
            fprintf(stderr, "Names: ");
            print_pyc_py_object(co->names, true);

            // Create names value array
            struct pyc_tuple *names_tuple = co->names->data;
            co->names_value = calloc(names_tuple->count, sizeof(PyObject));
            for(unsigned int i = 0; i < names_tuple->count; i++) {
                if(strcmp((char *) names_tuple->items[i]->data, "int") == 0
                        || strcmp((char *) names_tuple->items[i]->data, "input") == 0) {
                    co->names_value[i].type = TYPE_INTERNAL_NATIVEFUNCTION;
                    co->names_value[i].data = names_tuple->items[i]->data;
                }
                else if(strcmp((char *) names_tuple->items[i]->data, "__name__") == 0) {
                    co->names_value[i].type = TYPE_INTERNED;
                    co->names_value[i].data = "__main__";
                }
                else {
                    co->names_value[i].type = TYPE_NONE;
                }
            }

            // Read other objects
            /* fprintf(stderr, "-- START READING var_names\n"); */
            co->var_names = pyc_read_object(fh, obj);
            /* fprintf(stderr, "-- START READING free_vars\n"); */
            co->free_vars = pyc_read_object(fh, obj);
            /* fprintf(stderr, "-- START READING cell_vars\n"); */
            co->cell_vars = pyc_read_object(fh, obj);
            /* fprintf(stderr, "-- START READING filename\n"); */
            co->filename = pyc_read_object(fh, obj);
            /* fprintf(stderr, "-- START READING name\n"); */
            co->name = pyc_read_object(fh, obj);
            fread(&co->first_line_no, 4, 1, fh);
            /* fprintf(stderr, "-- START READING lnotab\n"); */
            co->lnotab = pyc_read_object(fh, obj);
        } break;
        case TYPE_TUPLE: {
            struct pyc_tuple *tuple = malloc(sizeof(struct pyc_tuple));
            obj->data = tuple;
            fread(&tuple->count, 4, 1, fh);
            /* fprintf(stderr, "Tuple length: %d\n", tuple->count); */
            tuple->items = calloc(tuple->count, sizeof(void *));
            for(unsigned int i = 0; i < tuple->count; i++) {
                tuple->items[i] = pyc_read_object(fh, obj);
            }
        } break;
        case TYPE_INTERNED:
        case TYPE_STRING: {
            uint32_t length;
            fread(&length, 4, 1, fh);
            char *str = malloc(length + 1);
            fread(str, length, 1, fh);
            str[length] = '\0';
            obj->data = str;
            /* fprintf(stderr, "Created string: '%s'\n", str); */
        } break;
        case TYPE_NONE:
        case TYPE_TRUE:
        case TYPE_FALSE:
            obj->data = NULL;
            break;
        case TYPE_STRING_REF:
        case TYPE_INT:
            obj->data = malloc(4);
            fread(obj->data, 4, 1, fh);
            break;
        default:
            fprintf(stderr, "Unknown PyObject type: 0x%02hhX\n", obj->type);
            obj->data = NULL;
            break;
    }

    if(obj->type == TYPE_INTERNED) {
        PyObject *interned_root = root;
        while(interned_root->root) {
            interned_root = interned_root->root;
        }

        CodeObject *root_co = interned_root->data;
        root_co->interned_strings[root_co->interned_strings_count] = obj;
        root_co->interned_strings_count++;
    }

    return obj;
}

PyObject *pyc_gen_string(char *str) {
    PyObject *obj = malloc(sizeof(PyObject));
    obj->type = TYPE_STRING;
    obj->data = str;
    return obj;
}

PyObject *pyc_gen_int(int i) {
    PyObject *obj = malloc(sizeof(PyObject));
    obj->type = TYPE_INT;
    obj->data = malloc(4);
    *(int *) obj->data = i;
    return obj;
}

PyObject *pyc_gen_none() {
    PyObject *obj = malloc(sizeof(PyObject));
    obj->type = TYPE_NONE;
    obj->data = NULL;
    return obj;
}

PyObject *pyc_gen_bool(bool b) {
    PyObject *obj = malloc(sizeof(PyObject));
    obj->type = b ? TYPE_TRUE : TYPE_FALSE;
    obj->data = NULL;
    return obj;
}

void pyc_free_tuple(struct pyc_tuple *tuple) {
    for(unsigned int i = 0; i < tuple->count; i++) {
        pyc_free_PyObject(tuple->items[i]);
    }

    free(tuple->items);
    free(tuple);
}

void pyc_free_Assembly(Assembly *assembly) {
    free(assembly->opargs);
    free(assembly);
}

void pyc_free_CodeObject(CodeObject *co) {
    pyc_free_Assembly(co->assembly);
    pyc_free_PyObject(co->consts);
    pyc_free_PyObject(co->names);
    free(co->names_value);
    pyc_free_PyObject(co->var_names);
    pyc_free_PyObject(co->free_vars);
    pyc_free_PyObject(co->cell_vars);
    pyc_free_PyObject(co->filename);
    pyc_free_PyObject(co->name);
    pyc_free_PyObject(co->lnotab);
    free(co);
}

void pyc_free_PyObject(PyObject *obj) {
    switch(obj->type) {
        case TYPE_TUPLE:
            pyc_free_tuple(obj->data);
            break;
        case TYPE_CODE_OBJECT:
            pyc_free_CodeObject(obj->data);
            break;
        case TYPE_NONE:
            break;
        default:
            free(obj->data);
            break;
    }

    free(obj);
}

int main(int argc, char **argv) {
    FILE *fh = fopen(argv[1], "rb");

    if(fh == NULL) {
        fprintf(stderr, "Could not open file\n");
        exit(EXIT_FAILURE);
    }

    struct pyc_header header;
    fread(&header, sizeof(struct pyc_header), 1, fh);
    print_pyc_header(&header);

    if(header.version_magic != 0xF303) {
        fprintf(stderr, "Invalid header\n");
        exit(EXIT_FAILURE);
    }

    PyObject *obj = pyc_read_object(fh, NULL);
    fprintf(stderr, "Execution starting!\n");
    pyc_execute(obj);
    pyc_free_PyObject(obj);

    fclose(fh);
    return 0;
}

