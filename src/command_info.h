#ifndef COMMAND_INFO_H
#define COMMAND_INFO_H

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
} Command;

Command *get_command_info(char *name);

#endif