#ifndef VARIABLES_H
#define VARIABLES_H

typedef struct variable
{
    char *name;
    char *value;
    struct variable *next; // Linked list for chaining collisions
} Variable;

typedef struct variable_table
{
    Variable **buckets; // Array of pointers to variable chains
    size_t capacity;    // Number of buckets
    size_t count;       // Total number of variables stored
} Variable_table;

#define INITIAL_CAPACITY 32

// Function declarations
Variable_table *create_table(size_t capacity);
void set_variable(Variable_table *table, const char *name, const char *value);
char *get_variable(Variable_table *table, const char *name);
void delete_variable(Variable_table *table, const char *name);
void free_table(Variable_table *table);

#endif
