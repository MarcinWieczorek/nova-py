#pragma once
#include <stdint.h>
#include <stdbool.h>

#define TYPE_TUPLE 0x28
#define TYPE_FALSE 0x46
#define TYPE_NONE 0x4E
#define TYPE_STRING_REF 0x52
#define TYPE_TRUE 0x54
#define TYPE_CODE_OBJECT 0x63
#define TYPE_INT 0x69
#define TYPE_STRING 0x73
#define TYPE_INTERNED 0x74
#define TYPE_UNICODE_STRING 0x75
#define TYPE_INTERNAL_NATIVEFUNCTION 0xA0

#define OP_LOAD_CONST 100

struct pyc_header {
    uint16_t version_magic;
    uint16_t crlf;
    uint32_t modification_timestamp;
}__attribute__((packed));

typedef struct pyc_op_arg {
    uint8_t  op_code;
    uint16_t arg;
    bool valid;
} OpArg;

typedef struct pyc_assembly {
    uint8_t string_magic;
    uint32_t length;
    uint32_t op_count;
    OpArg *opargs;
} Assembly;

typedef struct pyc_py_object {
    uint8_t type;
    void *data;
} PyObject;

typedef struct pyc_code_object {
    uint32_t arg_count;
    uint32_t local_count;
    uint32_t stack_size;
    uint32_t flags;

    uint32_t assembly_count;
    Assembly *assembly;
    PyObject *consts;
    PyObject *names;
    PyObject *names_value;
} CodeObject;


struct pyc_tuple {
    uint32_t count;
    struct pyc_py_object **items;
};

PyObject *pyc_gen_string(char *);

PyObject *pyc_gen_int(int);

PyObject *pyc_gen_bool(bool);

PyObject *pyc_gen_none();

void pyc_free_tuple(struct pyc_tuple *);

void pyc_free_Assembly(Assembly *);

void pyc_free_CodeObject(CodeObject *);

void pyc_free_PyObject(PyObject *);

