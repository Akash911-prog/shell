#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "commands.h"

#define VAR_IDENTIFIER '#'

void tokenize_cmd(char cmd[], char tokens[][50], int *no_of_tokens);

int main()
{
    setbuf(stdout, NULL);
    while (1)
    {
        // command token is tokens[0]. keeps this in mind. this word will be used a lot
        // command and prompt initialization
        char cmd[1024];
        cmd[0] = '\0';
        printf("$ ");

        if (fgets(cmd, sizeof(cmd), stdin) != NULL)
        {
            cmd[strcspn(cmd, "\n")] = '\0'; // removes \n from the the cmd string

            int no_of_tokens = 0;
            char tokens[50][50]; // tokens array contains all splited tokens

            tokenize_cmd(cmd, tokens, &no_of_tokens);

            // exit command
            if (strcmp(tokens[0], "exit") == 0)
            {
                exit(0);
            }

            // echo command
            if (strcmp(tokens[0], "echo") == 0)
            {
                // prints rest of the tokens except the command token
                echo(tokens, no_of_tokens);
                continue;
            }

            // type command
            if (strcmp(tokens[0], "type") == 0)
            {
                // prints the type of the command passed as argument
                type(tokens);
                continue;
            }

            if (tokens[0][0] == VAR_IDENTIFIER)
            {
                // Check if there's actually a variable name after $
                if (tokens[0][1] == '\0')
                {
                    printf("Error: no variable name specified\n");
                    continue;
                }

                // gets the variable name after #
                char *var = tokens[0] + 1;

                // gets the value of the variable as a pointer to the copied value of the env variable.
                // it uses malloc to free up the space after use.
                char *value = get_var(var);

                if (value != NULL)
                {
                    printf("%s\n", value);
                    free(value); // frees the malloc memory
                }
                else
                {
                    printf("%s: variable not found\n", var);
                }
                continue;
            }

            if (strcmp(tokens[0], "which") == 0)
            {
                char *file = find_file(tokens[1]);
                if (file != NULL)
                {
                    printf("%s\n", file);
                    free(file);
                    continue;
                }
                printf("%s not found\n", tokens[1]);
                continue;
            }

            printf("%s: cmd not found\n", tokens[0]);
        }
    }
    return 0;
}

void tokenize_cmd(char cmd[], char tokens[][50], int *no_of_tokens)
{
    int i = 0; // index of cmd
    int j = 0; // index of tokens
    int k = 0; // index inside current token

    int inside_quote = 0; // flag for inside_quotes state

    while (cmd[i] != '\0')
    {
        if (cmd[i] == '"')
        {
            inside_quote = !inside_quote;
            i++;
            continue;
        }

        // if whitespace and not inside quotes
        if ((cmd[i] == ' ') & !inside_quote)
        {
            // if index of token is > 0 then ends the ongoing token
            if (k > 0)
            {
                tokens[j][k] = '\0';
                j++;
                k = 0;
            }
            i++; // increase cmd index
            continue;
        }

        // adds the current cmd[i] to the current ongoing token
        tokens[j][k++] = cmd[i];
        i++;
    }

    // ends the last token if any
    if (k > 0)
    {
        tokens[j][k] = '\0';
        j++;
    }

    *no_of_tokens = j;
}