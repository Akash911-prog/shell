#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "command_info.h"

#define MAX_COMMANDS 30

// all commands metadata
Command command_metadata[] = {
    {.name = "echo", .type = BUILT_IN, .desc = "repeats the args", .help = "echo <string to print>", .argc = 1},
    {.name = "type", .type = BUILT_IN, .desc = "returns type of the function", .help = "type <command>", .argc = 1},
    {.name = "exit", .type = BUILT_IN, .desc = "closes the shell", .help = "exit", .argc = 0},
};

// array of pointer to the commands metadata
Command *commands[MAX_COMMANDS] = {
    &command_metadata[0],
    &command_metadata[1],
    &command_metadata[2],
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
