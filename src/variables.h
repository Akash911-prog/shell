// variables.h
#ifndef VARIABLES_H
#define VARIABLES_H

typedef struct variable
{
    char *name;
    char *value;
    struct variable *next;
} Variable;

#include <stddef.h>

typedef struct variable_table
{
    Variable **buckets;
    size_t capacity;
    size_t count;
} Variable_table;

// variables.h
typedef struct variableapi
{
    void (*init)(size_t capacity);
    void (*set)(char *name, char *value);
    char *(*get)(char *name);
    void (*delete)(char *name);
    void (*destroy)(); // Better name than delete_table
} VariableAPI;

// Global API instance
extern VariableAPI Variables;

#define INITIAL_CAPACITY 32

#endif