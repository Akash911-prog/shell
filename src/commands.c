#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "commands.h"
#include "builtins.h"
#include "utils.h"
#include "variables.h"
#include "lexer.h"

#ifdef _WIN32
#define CLEAR_COMMAND "cls"
#include <direct.h>
#define Chdir _chdir
#else
#define CLEAR_COMMAND "clear"
#include <unistd.h>
#define Chdir chdir
#endif

int dummy(TokenList *token_list)
// dummy placeholder function for the handler argument of exit command
{
    return 0;
}

int echo(TokenList *token_list)
{
    for (int i = 1; i < token_list->count; i++)
    {
        printf("%s ", token_list->tokens[i].raw);
        if (i == (token_list->count - 1))
        {
            printf("\n");
        }
    }
    return 0;
}

int type(TokenList *token_list)
{
    Command *cmd_info = get_command_info(token_list->tokens[1].raw); // gets command info: {name, type, desc, help, argc}. return null if cmd not found
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
    char *file = find_file(token_list->tokens[1].raw);
    if (file != NULL)
    {
        printf("%s\n", file);
        free(file);
        return 0;
    }
    printf("%s: not found\n", token_list->tokens[1].raw);
    return 1;
}

int which(TokenList *token_list)
{
    char *file = find_file(token_list->tokens[1].raw);
    if (file != NULL)
    {
        printf("%s\n", file);
        free(file);
        return 0;
    }
    printf("%s not found\n", token_list->tokens[1].raw);
    return 1;
}

int clear(TokenList *token_list)
{
    system(CLEAR_COMMAND);
    return 0;
}

int pwd(TokenList *token_list)
{
    printf("\nPath\n");
    printf("----\n");
    printf("%s\n\n\n", Variables.get("PWD"));
    return 0;
}

int cd(TokenList *token_list)
{
    // No argument - do nothing
    if (token_list->count < 2)
    {
        return 0;
    }

    char target_path[1024];
    char *arg = token_list->tokens[1].raw;

    // Handle ~ paths
    if (arg[0] == '~')
    {
        char *home = Variables.get("HOME");
        if (home == NULL)
        {
            fprintf(stderr, "cd: HOME not set\n");
            return 1;
        }

        if (arg[1] == '\0')
        {
            // Just "~"
            strcpy(target_path, home);
        }
        else if (arg[1] == '/')
        {
            // "~/path"
            snprintf(target_path, sizeof(target_path), "%s%s", home, arg + 1);
        }
        else
        {
            // Invalid like "~abc"
            strcpy(target_path, arg);
        }
    }
    else
    {
        // Use the path as-is (absolute or relative)
        strcpy(target_path, arg);
    }

    // Try to change directory
    if (Chdir(target_path) == 0)
    {
        set_pwd();
        init_prompt();
        return 0;
    }

    printf("cd: %s: No such file or directory\n", arg);
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

    value = Variables.get(var);

    if (value != NULL)
    {
        printf("%s\n", value);
        return 0;
    }

    printf("%s: Variable not found\n", var);
    return 1;
}

int execute(char *filepath, TokenList *token_list)
{
    if (token_list->count > 1)
    {
        char exec_string[1024];
        strcpy(exec_string, filepath);
        for (int i = 1; i < token_list->count; i++)
        {
            strcat(exec_string, " ");
            strcat(exec_string, token_list->tokens[i].raw);
        }
        system(exec_string);
        return 0;
    }
    system(filepath);
    return 0;
}