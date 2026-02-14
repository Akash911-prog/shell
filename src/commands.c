#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "commands.h"
#include "command_info.h"
#include "utils.h"

int dummy(char tokens[][50], int no_of_tokens)
// dummy placeholder function for the handler argument of exit command
{
    return 0;
}

int echo(char tokens[][50], int no_of_tokens)
{
    for (int i = 1; i < no_of_tokens; i++)
    {
        printf("%s ", tokens[i]);
        if (i == (no_of_tokens - 1))
        {
            printf("\n");
        }
    }
    return 0;
}

int type(char tokens[][50], int no_of_tokens)
{
    Command *cmd_info = get_command_info(tokens[1]); // gets command info: {name, type, desc, help, argc}. return null if cmd not found
    if (cmd_info != NULL)
    {
        if (cmd_info->type == BUILT_IN)
        {
            printf("BuiltIn\n");
        }
        else // Handle other types
        {
            printf("%s is an external command\n", cmd_info->name);
        }
        return 0;
    }
    char *file = find_file(tokens[1]);
    if (file != NULL)
    {
        printf("%s\n", file);
        free(file);
        return 0;
    }
    printf("%s: not found\n", tokens[1]);
    return 1;
}

int which(char tokens[][50], int no_of_tokens)
{
    char *file = find_file(tokens[1]);
    if (file != NULL)
    {
        printf("%s\n", file);
        free(file);
        return 0;
    }
    printf("%s not found\n", file);
    return 1;
}

int variable_handler(char var_name[])
{
    // Check if there's actually a variable name after $
    if (var_name[1] == '\0')
    {
        printf("Error: no variable name specified\n");
        return 0;
    }

    // gets the variable name after #
    char *var = var_name + 1;

    // gets the value of the variable as a pointer to the copied value of the env variable.
    // it uses malloc to free up the space after use.
    char *value = get_var(var);

    if (value != NULL)
    {
        printf("%s\n", value);
        free(value); // frees the malloc memory
        return 0;
    }
    else
    {
        printf("%s: variable not found\n", var);
    }
    return 1;
}

void execute(char *filepath)
{
    printf("EXECUTE\n");
}
