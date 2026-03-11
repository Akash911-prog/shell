#include "run.h"

#ifdef _WIN32

char *build_command_string(char *filepath, Node *node, IOContext *io)
{
    int total_len = strlen(filepath) + 1;
    for (int i = 1; i < node->arg_count; i++)
        total_len += strlen(node->args[i].raw) * 2 + 4;
    char *exec_string = malloc(total_len + 1);
    if (!exec_string)
    {
        fprintf(io->err, "malloc failed\n");
        return "";
    }

    strcpy(exec_string, filepath);

    if (node->arg_count > 1)
    {
        for (int i = 1; i < node->arg_count; i++)
        {
            strcat(exec_string, " \"");
            for (int j = 0; node->args[i].raw[j] != '\0'; j++)
            {
                if (node->args[i].raw[j] == '"')
                    strcat(exec_string, "\\\""); // escape inner quote
                else
                {
                    char ch[2] = {node->args[i].raw[j], '\0'};
                    strcat(exec_string, ch);
                }
            }
            strcat(exec_string, "\"");
        }
    }
    return exec_string; // caller frees
}

int run(char *filepath, Node *node, IOContext *io)
{
    // startup info for the proccess
    STARTUPINFO si = {sizeof(si)};
    si.dwFlags = STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW; // handles
    si.hStdInput = (HANDLE)_get_osfhandle(_fileno(io->in));
    si.hStdOutput = (HANDLE)_get_osfhandle(_fileno(io->out));
    si.hStdError = (HANDLE)_get_osfhandle(_fileno(io->err));
    si.wShowWindow = SW_HIDE;

    SECURITY_ATTRIBUTES sa = {sizeof(sa), NULL, TRUE}; // TRUE = inheritable

    PROCESS_INFORMATION pi;

    char *exec_string = build_command_string(filepath, node, io);

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

    free(exec_string);

    if (!ok)
    {
        DWORD err = GetLastError();
        printf("CreateProcess failed with error %lu\n", err);
        return 1;
    }

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
        fprintf(stderr, "wait failed with error %lu\n", err);
    }

    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
    return 0;
}

int run_piped_proccesses(char *filepath_left, Node *node_left,
                         char *filepath_right, Node *node_right,
                         IOContext *io)
{
    HANDLE hRead, hWrite;
    SECURITY_ATTRIBUTES sa = {sizeof(sa), NULL, TRUE}; // TRUE = inheritable

    // create the pipe with inheritable handles
    if (!CreatePipe(&hRead, &hWrite, &sa, 64 * 1024 * 1024)) // 64MB
    {
        fprintf(io->err, "CreatePipe failed: %lu\n", GetLastError());
        return 1;
    }

    // left process: stdout = hWrite
    STARTUPINFO si_left = {sizeof(si_left)};
    si_left.dwFlags = STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW;
    si_left.hStdInput = (HANDLE)_get_osfhandle(_fileno(io->in));
    si_left.hStdOutput = hWrite; // <-- writes into pipe
    si_left.hStdError = (HANDLE)_get_osfhandle(_fileno(io->err));
    si_left.wShowWindow = SW_HIDE;

    // right process: stdin = hRead
    STARTUPINFO si_right = {sizeof(si_right)};
    si_right.dwFlags = STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW;
    si_right.hStdInput = hRead; // <-- reads from pipe
    si_right.hStdOutput = (HANDLE)_get_osfhandle(_fileno(io->out));
    si_right.hStdError = (HANDLE)_get_osfhandle(_fileno(io->err));
    si_right.wShowWindow = SW_HIDE;

    PROCESS_INFORMATION pi_left, pi_right;

    // build command strings same way your run() does
    // ... (extract this into a helper to avoid duplication)
    char *cmd_left = build_command_string(filepath_left, node_left, io);
    BOOL ok_left = CreateProcess(NULL, cmd_left, NULL, NULL, TRUE, 0, NULL, NULL, &si_left, &pi_left);

    // CRITICAL: close hWrite in the parent after spawning left process
    // otherwise right process never gets EOF even when left exits
    CloseHandle(hWrite);

    char *cmd_right = build_command_string(filepath_right, node_right, io);
    BOOL ok_right = CreateProcess(NULL, cmd_right, NULL, NULL, TRUE, 0, NULL, NULL, &si_right, &pi_right);
    CloseHandle(hRead); // parent doesn't need this either

    // wait for both
    HANDLE procs[2] = {pi_left.hProcess, pi_right.hProcess};
    WaitForMultipleObjects(2, procs, TRUE, INFINITE);

    DWORD exit_code;
    GetExitCodeProcess(pi_right.hProcess, &exit_code); // typically care about right side's exit
    char buffer[12];
    snprintf(buffer, sizeof(buffer), "%lu", exit_code);
    Variables.set("?", buffer);

    CloseHandle(pi_left.hProcess);
    CloseHandle(pi_left.hThread);
    CloseHandle(pi_right.hProcess);
    CloseHandle(pi_right.hThread);
    free(cmd_left);
    free(cmd_right);
    return 0;
}

#else

// TODO: add run fucntion for unix
int run(char *filepath, Node *node, IOContext *io)
{
}

#endif