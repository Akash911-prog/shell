#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "commands.h"
#include "command_info.h"
#include "utils.h"

void echo(char tokens[][50], int no_of_tokens)
{
    for (int i = 1; i < no_of_tokens; i++)
    {
        printf("%s ", tokens[i]);
        if (i == (no_of_tokens - 1))
        {
            printf("\n");
        }
    }
}

int type(char tokens[][50])
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

char *get_var(const char *var)
{
    const char *env_v = getenv(var);
    if (env_v != NULL)
    {
        char *env_v_copy = malloc(strlen(env_v) + 1);
        if (env_v_copy != NULL)
        {
            strcpy(env_v_copy, env_v);
            return env_v_copy;
        }
    }
    return NULL;
}

char *find_file(char filename[])
{
    char *fullpath = get_var("PATH");

    if (fullpath != NULL)
    {
        char *filepath = recursive_file_search(fullpath, filename);
        if (filepath != NULL)
        {
            free(fullpath);
            return filepath;
        }

        free(fullpath);
        return NULL;
    }
}