#include "executor_win.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <windows.h>
#include <fileapi.h>
#include "commands.h"
#include "utils.h"
#include "variables.h"
#include "lexer.h"
#include "run.h"

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

void close_redirects(IOContext *io)
{
    if (io->close_out && io->out != stdout)
    {
        fflush(io->out);
        fclose(io->out);
        io->out = stdout;
        io->close_out = false;
    }

    if (io->close_in && io->in != stdin)
    {
        fclose(io->in);
        io->in = stdin;
        io->close_in = false;
    }

    if (io->close_err && io->err != stderr)
    {
        fflush(io->err);
        fclose(io->err);
        io->err = stderr;
        io->close_err = false;
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
        if (node->left->cmd_type == node->right->cmd_type)
        {
            if (node->left->cmd_type == BUILT_IN)
            {
                run_piped_builtin(io, node);
            }

            else if (node->left->cmd_type == EXTERNAL)
            {
                run_piped_proccesses(node->left->args[0].raw, node->left, node->right->args[0].raw, node->right, &io);
                // run both the proccesses in a os level pipe.
            }
        }
        else
        {
            run_piped_hybrid(io, node);
        }

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
        {
            close_redirects(&io);
            return;
        }

        char *filepath = find_file(node->args[0].raw);
        if (filepath != NULL)
        {
            run(filepath, node, &io);
            free(filepath);
            close_redirects(&io);
            return;
        }

        if (node->args[0].raw[0] == '.' &&
            (node->args[0].raw[1] == '/' || node->args[0].raw[1] == '\\'))
        {
            char buffer[1024];
            snprintf(buffer, sizeof(buffer), "%s%s%s", Variables.get("PWD"), PATH_SEP, (node->args[0].raw + 2));
            run(buffer, node, &io);
            return;
        }

        close_redirects(&io);

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