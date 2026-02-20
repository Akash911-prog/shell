#ifndef COMMAND_INFO_H
#define COMMAND_INFO_H

#include "lexer.h"

#define MAX_COMMANDS 30

typedef enum
{
    BUILT_IN
} Function_Type;

typedef struct
{
    char name[10];
    Function_Type type;
    char desc[50];
    char help[50];
    int argc;
    int (*handler)(TokenList *tl);
} Command;

extern Command command_metadata[];
extern Command *commands[MAX_COMMANDS];

Command *get_command_info(char *name);

#endif