#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>

#include "prints.h"

void print_pyc_header(struct pyc_header *h) {
    fprintf(stderr, "Version: %04X | CRLF: %04X | TS: %08X\n",
        h->version_magic,
        h->crlf,
        h->modification_timestamp);
}

void print_pyc_code_object(struct pyc_code_object *co) {
    fprintf(stderr, "CODEOBJ(AC:%d, LC:%d, SS:%d, LEN:%u)\n",
        co->arg_count,
        co->local_count,
        co->stack_size,
        co->assembly != NULL ? co->assembly->length : 0);
}

void print_pyc_py_object(struct pyc_py_object *obj, bool nl) {
    if(obj == NULL) {
        fprintf(stderr, "NULL");
        return;
    }

    switch(obj->type) {
        case TYPE_TUPLE: {
            struct pyc_tuple *tuple = obj->data;
            fprintf(stderr, "(");
            for(int i = 0; i < tuple->count; i++) {
                print_pyc_py_object(tuple->items[i], false);

                if(i < tuple->count - 1) {
                    fprintf(stderr, ", ");
                }
            }
            fprintf(stderr, ")");
            } break;
        case TYPE_CODE_OBJECT: {
            CodeObject *co = obj->data;
            fprintf(stderr, "<codeobject %p>", co);
            break;
        }
        case TYPE_STRING:
        case TYPE_INTERNED:
            fprintf(stderr, "'%s'", (char *) obj->data);
            break;
        case TYPE_STRING_REF: {
            int interned_string_index = *(int *) obj->data;
            PyObject *interned_root = obj->root;
            while(interned_root->root) {
                interned_root = interned_root->root;
            }

            CodeObject *root_co = interned_root->data;
            char *str = root_co->interned_strings[interned_string_index]->data;
            fprintf(stderr, "STRREF(#%d, '%s')", interned_string_index, str);
            break;
        }
        case TYPE_NONE:
            fprintf(stderr, "None");
            break;
        case TYPE_INT:
            fprintf(stderr, "%d", *(int *) obj->data);
            break;
        case TYPE_INTERNAL_NATIVEFUNCTION:
            fprintf(stderr, "<nfunction: '%s'>", (char *) obj->data);
            break;
        default:
            fprintf(stderr, "Can't print type: %02X", obj->type);
            break;
    }

    if(nl) {
        fprintf(stderr, "\n");
    }
}

// REPR functions
char *repr(PyObject *obj) {
    switch(obj->type) {
        case TYPE_STRING:
        case TYPE_INTERNED:
            return (char *) obj->data;
        case TYPE_STRING_REF: {
            int interned_string_index = *(int *) obj->data;
            PyObject *interned_root = obj->root;
            while(interned_root->root) {
                interned_root = interned_root->root;
            }
            CodeObject *root_co = interned_root->data;
            return root_co->interned_strings[interned_string_index]->data;
        }
        case TYPE_NONE:
            return "None";
        default:
            return "INVALID_REPR_TYPE";
    }
    return NULL;
}
