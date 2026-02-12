#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void tokenize_cmd(char cmd[], char tokens[][50], int *no_of_tokens);

int main()
{
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
                for (int i = 1; i < no_of_tokens; i++)
                {
                    printf("%s ", tokens[i]);
                    if (i == (no_of_tokens - 1))
                    {
                        printf("\n");
                    }
                }
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