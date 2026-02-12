#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void tokenize_cmd(char cmd[], char tokens[][50], int *no_of_tokens);

int main()
{
    while (1)
    {
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

            if (strcmp(tokens[0], "exit") == 0)
            {
                exit(0);
            }

            if (strcmp(tokens[0], "echo") == 0)
            {
                for (int i = 1; i < no_of_tokens - 1; i++)
                {
                    printf("%s ", tokens[i]);
                    if (i == (no_of_tokens - 2))
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
    // tokenize the string by making it in an array split by white spaces. same as pythons str.split()

    int i = 0;
    char temp[1024];

    // makes a temporary variable to not modify the actual cmd values
    strcpy(temp, cmd);

    // splitting logic. using strtok() look up the f**king documentation for strtok if you dont understand.
    char *tokenized_cmd = strtok(temp, " ");
    while (tokenized_cmd != NULL)
    {
        // copy's the n token in the nth index of tokens array that is passed by reference
        strcpy(tokens[i], tokenized_cmd);

        tokenized_cmd = strtok(NULL, " "); // gets the next token
        i++;
    }

    *no_of_tokens = i + 1; // len of tokens array. setting it to i + 1. as i is equal to the index of the array
}