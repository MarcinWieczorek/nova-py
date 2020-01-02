#include <stdio.h>
#include <stdlib.h>

#include "execute.h"
#include "novapy.h"

extern void (*opa[256])(Context *, uint16_t);
char *OP_LABELS[150];

PyObject *pyc_execute(PyObject *obj) {
    CodeObject *co = obj->data;
    Context c;
    c.stack = calloc(co->stack_size, sizeof(PyObject *));
    c.obj = obj;
    c.stack_pointer = 0;
    c.ip = 0;

    /* for(; c.ip < co->assembly->op_count; c.ip++) { */
    for(; c.ip < co->assembly->length; c.ip++) {
        OpArg *op = co->assembly->opargs + c.ip;
        if(!op->valid) {
            continue;
        }

        fprintf(stderr, "%-3i %-3d 0x%02X %-20s %04X", c.ip,
            op->op_code, op->op_code,
            OP_LABELS[op->op_code], op->arg);

        if(opa[op->op_code] != NULL) {
            opa[op->op_code](&c, op->arg);
            fprintf(stderr, "\n");
        }
        else {
            fprintf(stderr, "    not implemented\n");
        }

        if(op->op_code == 0x53) { // RETURN_VALUE
            free(c.stack);
            return c.ret;
        }
    }

    free(c.stack);
    return NULL;
}

void context_stack_push(Context *c, PyObject *obj) {
    c->stack[c->stack_pointer++] = obj;
}

PyObject *context_stack_pop(Context *c) {
    if(c->stack_pointer <= 0) {
        fprintf(stderr, "\nStack corrupted.\n");
        exit(EXIT_FAILURE);
    }
    return c->stack[--c->stack_pointer];
}

PyObject *context_stack_top(Context *c) {
    if(c->stack_pointer <= 0) {
        fprintf(stderr, "\nStack corrupted.\n");
        exit(EXIT_FAILURE);
    }
    return c->stack[c->stack_pointer - 1];
}

