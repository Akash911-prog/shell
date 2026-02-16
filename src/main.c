#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "command_info.h"
#include "commands.h"
#include "utils.h"
#include "variables.h"

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
        // command token is tokens[0]. keeps this in mind. this word will be used a lot
        // command and prompt initialization
        char cmd[1024];
        cmd[0] = '\0';
        printf("%s", Variables.get("PS1"));

        if (fgets(cmd, sizeof(cmd), stdin) != NULL) // takes input from stdin
        {
            printf("\033[0m");
            cmd[strcspn(cmd, "\n")] = '\0'; // removes \n from the the cmd string

            int no_of_tokens = 0;
            char tokens[50][50]; // tokens array contains all splited tokens

            tokenize_cmd(cmd, tokens, &no_of_tokens);
            int command_found = 0;

            int i = 0;

            if (strcmp(tokens[0], "exit") == 0)
            {
                exit(0);
            }
            if (tokens[0][0] == VAR_IDENTIFIER) // if its a variable
            {
                variable_handler(tokens[0]);
                continue;
            }
            // loops through all commands metadata and compares there name with the inputed command.
            while (commands[i] != NULL)
            {
                if (strcmp(tokens[0], commands[i]->name) == 0)
                {
                    command_found = 1;
                    commands[i]->handler(tokens, no_of_tokens); // calls the handler function associated with the command. see commands_info.h for detailed description
                    break;
                }
                i++;
            }
            if (command_found == 0)
            {
                char *filepath = find_file(tokens[0]); // return a file path string in malloc memory. so free it.
                if (filepath != NULL)
                {
                    execute(filepath, tokens, no_of_tokens);
                    free(filepath);
                    continue;
                }

                if ((tokens[0][0] == '.') && (tokens[0][1] == '/' || tokens[0][1] == '\\')) // checks if the command in current directory executable
                {
                    char buffer[1024];
                    snprintf(buffer, sizeof(buffer), "%s%s%s", Variables.get("CWD"), PATH_SEP, (tokens[0] + 2));
                    system(buffer);
                    continue;
                }

                // if nothing found. neither a command nor a executable.
                printf("%s: not found\n", tokens[0]);
            }
        }
    }
    return 0;
}
