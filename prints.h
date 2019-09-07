#pragma once
#include "novapy.h"

void print_pyc_header(struct pyc_header *);

void print_pyc_code_object(struct pyc_code_object *);

void print_pyc_py_object(struct pyc_py_object *, bool nl);

