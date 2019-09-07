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
    fprintf(stderr, "AC: %d, LC: %d, SS: %d, LEN: %u\n",
        co->arg_count,
        co->local_count,
        co->stack_size,
        co->assembly->length);
}

void print_pyc_py_object(struct pyc_py_object *obj, bool nl) {
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
        case TYPE_CODE_OBJECT:
            print_pyc_code_object(obj->data);
            break;
        case TYPE_STRING:
        case TYPE_INTERNED:
            fprintf(stderr, "'%s'", (char *) obj->data);
            break;
        case TYPE_STRING_REF:
            fprintf(stderr, "STRREF #%d", *(int *) obj->data);
            break;
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

