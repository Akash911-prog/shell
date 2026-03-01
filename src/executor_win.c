#include "executor_win.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "commands.h"
#include "utils.h"
#include "variables.h"

void execute_win(Node *node)
{
    if (node == NULL)
    {
        return;
    }

    execute_win(node->left);
    switch (node->type)
    {
    case CMD_LINE:
        break;

    case PIPE:
        break;

    case CMD:
        bool command_found = find_and_run_builtin(node);
        if (command_found)
        {
            return;
        }
        /* External executable on PATH */
        char *filepath = find_file(node->args[0].raw);
        if (filepath != NULL)
        {
            run(filepath, node);
            free(filepath);
            return;
        }

        /* Relative path: ./foo */
        if (node->args[0].raw[0] == '.' &&
            (node->args[0].raw[1] == '/' || node->args[0].raw[1] == '\\'))
        {
            char buffer[1024];
            snprintf(buffer, sizeof(buffer), "%s%s%s", Variables.get("PWD"), PATH_SEP, (node->args[0].raw + 2)); // makes a path to the executable
            system(buffer);
            return;
        }

        printf("%s: not found\n", node->args[0].raw);
        return;
        break;

    case SUB_COMMANDS:
        break;

    case AND:
        break;

    case OR:
        break;

    case ASSIGNMENT:
        break;

    default:
        break;
    }
    execute_win(node->right);
    execute_win(node->body);
}