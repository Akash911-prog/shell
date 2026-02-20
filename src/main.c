#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "builtins.h"
#include "commands.h"
#include "utils.h"
#include "variables.h"
#include "lexer.h"

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

        TokenList token_list = {0};
        lex(cmd, &token_list);

        if (token_list.count == 0)
            continue;

        Token *cmd_token = &token_list.tokens[0];

        /* Debug: dump tokens */
        if (strcmp(cmd_token->raw, "lex") == 0)
        {
            for (int i = 0; i < token_list.count; i++)
                printf("[%d] raw='%s' value='%s' type=%d\n",
                       i,
                       token_list.tokens[i].raw,
                       token_list.tokens[i].value,
                       token_list.tokens[i].type);
            continue;
        }

        /* Variable query: $VAR */
        if (cmd_token->raw[0] == VAR_IDENTIFIER)
        {
            variable_handler(cmd_token->raw);
            continue;
        }

        /* Built-in commands */
        int command_found = 0;
        for (int i = 0; commands[i] != NULL; i++)
        {
            if (strcmp(cmd_token->raw, commands[i]->name) == 0)
            {
                command_found = 1;
                commands[i]->handler(&token_list);
                break;
            }
        }

        if (command_found)
            continue;

        /* External executable on PATH */
        char *filepath = find_file(cmd_token->raw);
        if (filepath != NULL)
        {
            execute(filepath, &token_list);
            free(filepath);
            continue;
        }

        /* Relative path: ./foo */
        if (cmd_token->raw[0] == '.' &&
            (cmd_token->raw[1] == '/' || cmd_token->raw[1] == '\\'))
        {
            char buffer[1024];
            snprintf(buffer, sizeof(buffer), "%s%s%s",
                     Variables.get("CWD"), PATH_SEP, (cmd_token->raw + 2));
            system(buffer);
            continue;
        }

        printf("%s: not found\n", cmd_token->raw);
    }

    return 0;
}