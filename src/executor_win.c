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
#include <fcntl.h> // _O_WRONLY, _O_RDONLY
#include <io.h>    // _open_osfhandle, _fdopen

#define MAX_PIPE_BUFFER_SIZE 64 * 1000 * 1000 // 64 MB / megabytes

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

void make_pipe(IOContext *left_io, IOContext *right_io)
{
    char pipe_name[256];
    static int pipe_id = 0;
    snprintf(pipe_name, sizeof(pipe_name), "\\\\.\\pipe\\_%d_%d", GetCurrentProcessId(), pipe_id++);

    HANDLE hRead = CreateNamedPipe(
        pipe_name,
        PIPE_ACCESS_INBOUND,
        PIPE_TYPE_BYTE | PIPE_READMODE_BYTE | PIPE_WAIT,
        1,
        MAX_PIPE_BUFFER_SIZE,
        MAX_PIPE_BUFFER_SIZE,
        0,
        NULL);

    HANDLE hWrite = CreateFile(
        pipe_name,
        GENERIC_WRITE,
        0,
        NULL,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        NULL);

    left_io->out = _fdopen(_open_osfhandle((intptr_t)hWrite, _O_WRONLY), "w");
    right_io->in = _fdopen(_open_osfhandle((intptr_t)hRead, _O_RDONLY), "r");
    left_io->close_out = true;
    right_io->close_in = true;
}

typedef struct
{
    Node *node;
    IOContext io;
} BuiltinThreadArgs;

DWORD WINAPI builtin_thread_wrapper(LPVOID lpParam)
{
    BuiltinThreadArgs *args = (BuiltinThreadArgs *)lpParam;
    find_and_run_builtin(args->node, args->io);
    if (args->io.close_out && args->io.out != stdout)
        fclose(args->io.out); // signal EOF to the reader thread
    return 0;
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
        switch (node->left->cmd_type == node->right->cmd_type)
        {
        case true:
            if (node->left->cmd_type == BUILT_IN)
            {
                IOContext left_io = io;
                IOContext right_io = io;

                make_pipe(&left_io, &right_io);

                BuiltinThreadArgs left_args = {node->left, left_io};
                BuiltinThreadArgs right_args = {node->right, right_io};

                HANDLE threads[2];
                threads[0] = CreateThread(NULL, 0, builtin_thread_wrapper, &left_args, 0, NULL);
                threads[1] = CreateThread(NULL, 0, builtin_thread_wrapper, &right_args, 0, NULL);
                WaitForMultipleObjects(2, threads, TRUE, INFINITE);
                CloseHandle(threads[0]);
                CloseHandle(threads[1]);
            }

            else if (node->left->cmd_type == EXTERNAL)
            {
                run_piped_proccesses(node->left->args[0].raw, node->left, node->right->args[0].raw, node->right, &io);
                // run both the proccesses in a os level pipe.
            }

            break;

        case false:
            break;

        default:
            break;
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