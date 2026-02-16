// variables.c
#include "variables.h"
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#define FNV_PRIME 1099511628211ULL
#define FNV_OFFSET_BASIS 14695981039346656037ULL

// Global table (hidden from users)
static Variable_table *global_table = NULL;

static size_t hash(const char *data, size_t capacity)
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

// Implementation functions
static void _init(size_t capacity)
{
    if (global_table != NULL)
    {
        // Already initialized, maybe free first?
        return;
    }
    global_table = malloc(sizeof(Variable_table));
    global_table->buckets = calloc(capacity, sizeof(Variable *));
    global_table->capacity = capacity;
    global_table->count = 0;
}

static void _set(char *name, char *value)
{
    if (global_table == NULL)
        return; // Not initialized

    size_t index = hash(name, global_table->capacity);
    Variable *current = global_table->buckets[index];

    while (current != NULL)
    {
        if (strcmp(current->name, name) == 0)
        {
            free(current->value);
            current->value = strdup(value);
            return;
        }
        current = current->next;
    }

    Variable *new_var = malloc(sizeof(Variable));
    new_var->name = strdup(name);
    new_var->value = strdup(value);
    new_var->next = global_table->buckets[index];
    global_table->buckets[index] = new_var;
    global_table->count++;
}

static char *_get(char *name)
{
    if (global_table == NULL)
        return NULL;

    size_t index = hash(name, global_table->capacity);
    Variable *current = global_table->buckets[index];

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

static void _delete(char *name)
{
    if (global_table == NULL)
        return;

    size_t index = hash(name, global_table->capacity);
    Variable *current = global_table->buckets[index];
    Variable *prev = NULL;

    while (current != NULL)
    {
        if (strcmp(current->name, name) == 0)
        {
            if (prev == NULL)
            {
                global_table->buckets[index] = current->next;
            }
            else
            {
                prev->next = current->next;
            }
            free(current->name);
            free(current->value);
            free(current);
            global_table->count--;
            return;
        }
        prev = current;
        current = current->next;
    }
}

static void _destroy()
{
    if (global_table == NULL)
        return;

    for (size_t i = 0; i < global_table->capacity; i++)
    {
        Variable *current = global_table->buckets[i];
        while (current != NULL)
        {
            Variable *next = current->next;
            free(current->name);
            free(current->value);
            free(current);
            current = next;
        }
    }
    free(global_table->buckets);
    free(global_table);
    global_table = NULL;
}

// Global API instance
VariableAPI Variables = {
    .init = _init,
    .set = _set,
    .get = _get,
    .delete = _delete,
    .destroy = _destroy};