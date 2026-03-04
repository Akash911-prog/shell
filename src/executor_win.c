#include "executor_win.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <windows.h>
#include "commands.h"
#include "utils.h"
#include "variables.h"
#include "lexer.h"

// default
IOContext default_io()
{
    return (IOContext){stdin, stdout, stderr, false, false, false};
}

void apply_redirects(IOContext *io, Node *node)
{
    if (node->redirect_count <= 0)
        return;

    for (int i = 0; i < node->redirect_count; i++)
    {
        Redirect r = node->redirects[i];

        switch (r.type)
        {
        case TOKEN_REDIRECT_IN: //
            if (io->close_in)
                fclose(io->in); // close previous if we opened it
            io->in = fopen(r.filename, "r");
            io->close_in = true;
            break;

        case TOKEN_REDIRECT_OUT: // >
            if (io->close_out)
                fclose(io->out);
            io->out = fopen(r.filename, "w");
            io->close_out = true;
            break;

        case TOKEN_REDIRECT_APPEND: // >>
            if (io->close_out)
                fclose(io->out);
            io->out = fopen(r.filename, "a");
            io->close_out = true;
            break;

        case TOKEN_REDIRECT_ERR: // 2>
            if (io->close_err)
                fclose(io->err);
            io->err = fopen(r.filename, "w");
            io->close_err = true;
            break;

        case TOKEN_REDIRECT_ERR_APPEND: // 2>>
            if (io->close_err)
                fclose(io->err);
            io->err = fopen(r.filename, "a");
            io->close_err = true;
            break;
        }

        if (!io->in || !io->out || !io->err)
        {
            fprintf(stderr, "%s: no such file or directory\n", r.filename);
            return;
        }
    }
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
        if (node->redirect_count > 0)
        {
            apply_redirects(&io, node);
        }
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