#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "command_info.h"
#include "commands.h"

// all commands metadata
Command command_metadata[] = {
    {.name = "echo", .type = BUILT_IN, .desc = "repeats the args", .help = "echo <string to print>", .argc = 1, .handler = echo},
    {.name = "type", .type = BUILT_IN, .desc = "returns type of the function", .help = "type <command>", .argc = 1, .handler = type},
    {.name = "which", .type = BUILT_IN, .desc = "searches a specific executable in the path", .help = "which <executable_to_search>", .argc = 1, .handler = which},
    {.name = "clear", .type = BUILT_IN, .desc = "clears all visible text in the terminal", .help = "clear", .argc = 0, .handler = clear},
    {.name = "pwd", .type = BUILT_IN, .desc = "prints the current working directory", .help = "pwd", .argc = 0, .handler = pwd},
    {.name = "cd", .type = BUILT_IN, .desc = "changes the current woking directory", .help = "cd <path> // path can be absolute/relative/~", .argc = 1, .handler = cd},
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
