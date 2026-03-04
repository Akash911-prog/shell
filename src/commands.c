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

int dummy(Node *node, IOContext io)
// dummy placeholder function for the handler argument of exit command
{
    return 0;
}

// echos io.output to a FILE * io.out
int echo(Node *node, IOContext io)
{
    char buff[2048];
    int offset = 0;

    if (node->arg_count == 1)
    {
        if (node->args[0].raw[0] == '$')
        {
            snprintf(buff + offset, sizeof(buff) - offset, "%s\n", get_Token_value(&(node->args[0])));
        }
    }

    for (int i = 1; i < (node->arg_count); i++)
    {
        offset += snprintf(buff + offset, sizeof(buff) - offset, "%s", get_Token_value(&(node->args[i])));
        if (i == (node->arg_count - 1))
        {
            snprintf(buff + offset, sizeof(buff) - offset, "%s", "\n");
        }
    }

    fprintf(io.out, "%s", buff);
    fflush(io.out);
    if (io.out == stdout)
    {
        return 0;
    }
    fclose(io.out);
    return 0;
}

int type(Node *node, IOContext io)
{
    Command *cmd_info = get_command_info(get_Token_value(&node->args[1])); // gets command info: {name, type, desc, help, argc}. return null if cmd not found
    if (cmd_info != NULL)
    {
        if (cmd_info->type == BUILT_IN)
        {
            fprintf(io.out, "BuiltIn\n");
        }
        else // Handle other types
        {
            fprintf(io.out, "%s is an external command\n", cmd_info->name);
        }
        return 0;
    }
    char *file = find_file(get_Token_value(&node->args[1]));
    if (file != NULL)
    {
        fprintf(io.out, "%s\n", file);
        free(file);
        fflush(io.out);
        if (io.out == stdout)
        {
            return 0;
        }
        fclose(io.out);
        return 0;
    }
    fprintf(io.err, "%s: not found\n", get_Token_value(&node->args[1]));
    return 1;
}

int which(Node *node, IOContext io)
{
    char *file = find_file(get_Token_value(&node->args[1]));
    if (file != NULL)
    {
        fprintf(io.out, "%s\n", file);
        free(file);
        return 0;
    }
    fprintf(io.err, "%s not found\n", get_Token_value(&node->args[1]));
    return 1;
}

int clear(Node *node, IOContext io)
{
    system(CLEAR_COMMAND);
    return 0;
}

int pwd(Node *node, IOContext io)
{
    fprintf(io.out, "\nPath\n----\n%s\n\n\n", Variables.get("PWD"));
    return 0;
}

int cd(Node *node, IOContext io)
{
    // No argument - do nothing
    if (node->arg_count < 2)
    {
        return 0;
    }

    char target_path[1024];
    char *arg = get_Token_value(&node->args[1]);

    // Handle ~ paths
    if (arg[0] == '~')
    {
        char *home = Variables.get("HOME");
        if (home == NULL)
        {
            fprintf(io.err, "cd: HOME not set\n");
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

    fprintf(io.err, "cd: %s: No such file or directory\n", arg);
    return 1;
}

int cat(Node *node, IOContext io)
{
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

int run(char *filepath, Node *node)
{
    if (node->arg_count > 1)
    {
        char exec_string[1024];
        strcpy(exec_string, filepath);
        for (int i = 1; i < node->arg_count; i++)
        {
            strcat(exec_string, " ");
            strcat(exec_string, node->args[i].raw);
        }
        system(exec_string);
        return 0;
    }
    system(filepath);
    return 0;
}