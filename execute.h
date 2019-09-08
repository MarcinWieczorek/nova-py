#pragma once
#include "novapy.h"

typedef struct pyc_execute_context {
    PyObject **stack;
    uint32_t stack_pointer;
    uint32_t ip; // Instruction Pointer
    PyObject *obj;
    PyObject *ret;
} Context;

PyObject *pyc_execute(PyObject *);

void context_stack_push(Context *, PyObject *);

PyObject *context_stack_pop(Context *);

PyObject *context_stack_top(Context *);

