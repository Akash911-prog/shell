#ifndef AST_H
#define AST_H

#include "lexer.h"
#include <stdbool.h>

typedef enum
{
    CMD_LINE,
    PIPE,
    CMD,
    SUB_COMMANDS,
    AND,
    OR,
} NodeType;

typedef struct
{
    char *filename; // File to redirect to/from
    TokenType type; // REDIRECT_IN, REDIRECT_OUT, REDIRECT_APPEND
    int fd;         // Which fd: 0=stdin, 1=stdout, 2=stderr
} Redirect;

typedef struct Node
{
    NodeType type; // node type

    // For NODE_COMMAND (leaf nodes)
    char **args;         // Array of argument strings
    int arg_count;       // Number of arguments
    Redirect *redirects; // Array of redirects
    int redirect_count;  // Number of redirects

    // For binary nodes (PIPE, AND, OR, SEQUENCE)
    struct Node *left;  // ← Left child pointer
    struct Node *right; // ← Right child pointer

    // For unary nodes (SUBSHELL)
    struct Node *body; // ← Single child pointer

    // Metadata
    bool background; // Run in background?

    void (*add_arg)(struct Node *self, char *args);
    void (*add_redirect)(struct Node *self, Redirect redirect);

} Node;

Node *new_node(NodeType type);

#endif