#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "builtins.h"
#include "commands.h"
#include "lexer.h"

// all commands metadata
Command command_metadata[] = {
    {.name = "echo", .type = BUILT_IN, .desc = "repeats the args", .help = "echo <string to print>", .argc = 1, .handler = echo},
    {.name = "type", .type = BUILT_IN, .desc = "returns type of the function", .help = "type <command>", .argc = 1, .handler = type},
    {.name = "which", .type = BUILT_IN, .desc = "searches a specific executable in the path", .help = "which <executable_to_search>", .argc = 1, .handler = which},
    {.name = "clear", .type = BUILT_IN, .desc = "clears all visible text in the terminal", .help = "clear", .argc = 0, .handler = clear},
    {.name = "pwd", .type = BUILT_IN, .desc = "prints the current working directory", .help = "pwd", .argc = 0, .handler = pwd},
    {.name = "cd", .type = BUILT_IN, .desc = "changes the current woking directory", .help = "cd <path> // path can be absolute/relative/~", .argc = 1, .handler = cd},
    {.name = "ls", .type = BUILT_IN, .desc = "list all files", .help = "ls", .argc = 0, .handler = ls},
    {.name = "exit", .type = BUILT_IN, .desc = "closes the shell", .help = "exit", .argc = 0, .handler = dummy},
};

// array of pointer to the commands metadata
Command *commands[MAX_COMMANDS] = {
    &command_metadata[0],
    &command_metadata[1],
    &command_metadata[2],
    &command_metadata[3],
    &command_metadata[4],
    &command_metadata[5],
    &command_metadata[6],
    &command_metadata[7],
};

Command *get_command_info(char *name)
{
    // traverses the command dat
    int i = 0;
    while (commands[i] != NULL)
    {
        if (strcmp((commands[i]->name), name) == 0)
        {
            return commands[i];
        }
        i++;
    }
    return NULL;
}

bool find_and_run_builtin(Node *node, IOContext io)
{
    bool command_found = false;
    for (int i = 0; commands[i] != NULL; i++)
    {
        if (strcmp(node->args[0].raw, commands[i]->name) == 0)
        {
            command_found = true;
            commands[i]->handler(node, io);
            break;
        }
    }
    if (!command_found && (node->args[0].raw[0]) == '$')
    {
        echo(node, io);
        return true;
    }
    return command_found;
}

bool is_builtin(char *cmd_name)
{
    for (int i = 0; commands[i] != NULL; i++)
    {
        if (strcmp(cmd_name, commands[i]->name) == 0)
        {
            return true;
        }
    }
    return false;
}