#include "run.h"

#ifdef _WIN32

int run(char *filepath, Node *node, IOContext *io)
{
    // startup info for the proccess
    STARTUPINFO si = {sizeof(si)};
    si.dwFlags = STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW; // handles
    si.hStdInput = io->in;
    si.hStdOutput = io->out;
    si.hStdError = io->err;
    si.wShowWindow = SW_HIDE;

    SECURITY_ATTRIBUTES sa = {sizeof(sa), NULL, TRUE}; // TRUE = inheritable

    PROCESS_INFORMATION pi;

    int total_len = strlen(filepath) + 1;
    for (int i = 1; i < node->arg_count; i++)
        total_len += strlen(node->args[i].raw) + 1;

    char *exec_string = malloc(total_len);

    strcpy(exec_string, filepath);

    if (node->arg_count > 1)
    {
        for (int i = 1; i < node->arg_count; i++)
        {
            strcat(exec_string, " ");
            strcat(exec_string, node->args[i].raw);
        }
    }

    BOOL ok = CreateProcess(
        NULL,        // [1] executable name (NULL = parse from lpCommandLine)
        exec_string, // [2] command line string (e.g. L"grep foo")
        NULL,        // [3] process security attributes (NULL = default)
        NULL,        // [4] thread security attributes  (NULL = default)
        TRUE,        // [5] inherit handles? TRUE = child inherits open handles
        0,           // [6] creation flags
        NULL,        // [7] environment block (NULL = inherit parent's)
        NULL,        // [8] working directory (NULL = inherit parent's)
        &si,         // [9] STARTUPINFO we filled out
        &pi          // [10] OUTPUT: gets process/thread handles + IDs
    );

    DWORD result = WaitForSingleObject(pi.hProcess, INFINITE);

    if (result == WAIT_OBJECT_0)
    {
        // process finished, safe to get exit code
        DWORD exit_code;
        GetExitCodeProcess(pi.hProcess, &exit_code);
        char buffer[12];
        snprintf(buffer, sizeof(buffer), "%lu", exit_code);
        Variables.set("?", buffer);
    }
    else if (result == WAIT_FAILED)
    {
        // something went wrong
        DWORD err = GetLastError();
        printf("wait failed with error %lu\n", err);
    }

    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
    return 0;
}

#else

int run(char *filepath, Node *node, IOContext *io)
{
}

#endif