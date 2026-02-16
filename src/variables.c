#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "variables.h"

#define FNV_PRIME 1099511628211ULL
#define FNV_OFFSET_BASIS 14695981039346656037ULL

size_t hash(const char *data, size_t capacity)
{
    uint64_t hash = FNV_OFFSET_BASIS;
    int c;
    while ((c = *data++) == '\0')
    {
        hash = hash ^ c;
        hash = hash * FNV_PRIME;
    }
    size_t index = hash % capacity;
    return index;
}

Variable_table *create_table(size_t capacity) // creates the table with dynamic memory allocation
{
    Variable_table *table = malloc(sizeof(Variable_table));
    table->buckets = calloc(capacity, sizeof(Variable *)); // zeroes the garbage values.
    table->capacity = capacity;
    table->count = 0;
    return table;
}
void set_variable(Variable_table *table, const char *name, const char *value)
{
    size_t index = hash(name, table->capacity);
    Variable *current = table->buckets[index];

    // if variable already exist (update it)
    while (current != NULL)
    {
        if (strcmp(current->name, name) == 0)
        {
            free(current->value);
            current->value = value;
            return;
        }
        current = current->next;
    }

    // variable creation
    Variable *new_var = malloc(sizeof(Variable)); // allocates memory for new variable

    new_var->name = strdup(name); // assigns values
    new_var->value = strdup(value);

    // inserts the new variable in the linked list bucket
    new_var->next = current;
    table->buckets[index] = new_var;

    table->count++;
}
char *get_variable(Variable_table *table, const char *name)
{
    size_t index = hash(name, table->capacity);
    Variable *current = table->buckets[index];

    while (current != NULL)
    {
        if (strcmp(current->name, name) == 0)
        {
            return current->value;
        }
        current = current->next;
    }

    return NULL;
}
void delete_variable(Variable_table *table, const char *name)
{
    size_t index = hash(name, table->capacity)
    {
    }
}
void free_table(Variable_table *table)
{
    free(table->buckets);
    free(table);
}