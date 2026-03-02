#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "builtins.h"
#include "commands.h"
#include "utils.h"
#include "variables.h"
#include "lexer.h"
#include "tokenization_pipeline.h"

#define VAR_IDENTIFIER '$'
#ifdef _WIN32
#define PATH_SEP "\\"
#else
#define PATH_SEP "/"
#endif

int main()
{
    Variables.init(32);
    setbuf(stdout, NULL);
    init_prompt();

    while (1)
    {
        char cmd[1024];
        cmd[0] = '\0';
        printf("%s", Variables.get("PS1"));

        if (fgets(cmd, sizeof(cmd), stdin) == NULL)
            continue;

        printf("\033[0m");
        cmd[strcspn(cmd, "\n")] = '\0';

        if (strcmp(cmd, "exit") == 0)
            exit(0);

        exec_input(cmd);
    }

    return 0;
}