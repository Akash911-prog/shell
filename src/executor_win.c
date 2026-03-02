#include "executor_win.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <windows.h>
#include "commands.h"
#include "utils.h"
#include "variables.h"

// default
IOContext default_io()
{
    return (IOContext){stdin, stdout, stderr};
}

void execute_win(Node *node, IOContext io)
{
    if (node == NULL)
        return;

    switch (node->type)
    {
    case CMD_LINE:
        execute_win(node->left, io);
        execute_win(node->right, io);
        break;

    case PIPE:
    {
        HANDLE read_h, write_h;
        SECURITY_ATTRIBUTES sa = {sizeof(sa), NULL, TRUE};
        CreatePipe(&read_h, &write_h, &sa, 0);

        FILE *write_f = _fdopen(_open_osfhandle((intptr_t)write_h, 0), "w");
        FILE *read_f = _fdopen(_open_osfhandle((intptr_t)read_h, 0), "r");

        IOContext left_io = {io.in, write_f, io.err};
        execute_win(node->left, left_io);
        fclose(write_f); // signals EOF to right side

        IOContext right_io = {read_f, io.out, io.err};
        execute_win(node->right, right_io);
        fclose(read_f);
        break;
    }

    case CMD:
    {
        bool command_found = find_and_run_builtin(node, io);
        if (command_found)
            return;

        char *filepath = find_file(node->args[0].raw);
        if (filepath != NULL)
        {
            run(filepath, node);
            free(filepath);
            return;
        }

        if (node->args[0].raw[0] == '.' &&
            (node->args[0].raw[1] == '/' || node->args[0].raw[1] == '\\'))
        {
            char buffer[1024];
            snprintf(buffer, sizeof(buffer), "%s%s%s", Variables.get("PWD"), PATH_SEP, (node->args[0].raw + 2));
            system(buffer);
            return;
        }

        printf("%s: not found\n", node->args[0].raw);
        break;
    }

    case SUB_COMMANDS:
        execute_win(node->body, io);
        break;

    case AND:
        execute_win(node->left, io);
        execute_win(node->right, io);
        break;

    case OR:
        execute_win(node->left, io);
        execute_win(node->right, io);
        break;

    case ASSIGNMENT:
        // TODO: handle assignment
        break;

    default:
        execute_win(node->left, io);
        execute_win(node->right, io);
        execute_win(node->body, io);
        break;
    }
}