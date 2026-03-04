#ifndef COMMAND_INFO_H
#define COMMAND_INFO_H

#include "lexer.h"
#include "ast.h"
#include <stdbool.h>
#include <stdio.h>
#include "iocontext.h"

#define MAX_COMMANDS 30

typedef enum
{
    BUILT_IN,
    EXTERNAL
} Function_Type;

typedef struct
{
    char name[10];
    Function_Type type;
    char desc[50];
    char help[50];
    int argc;
    int (*handler)(Node *node, IOContext io);
} Command;

extern Command command_metadata[];
extern Command *commands[MAX_COMMANDS];

Command *get_command_info(char *name);
bool find_and_run_builtin(Node *node, IOContext io);

#endif