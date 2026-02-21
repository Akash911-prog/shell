#include "expander.h"
#include <string.h>
#include <ctype.h>
#include <stdio.h>

static void expand_arg(Token *arg)
{
    char result[256] = "";
    char *str = arg->raw;

    for (size_t k = 0; k < strlen(str); k++)
    {
        if (str[k] == '$')
        {
            k++; // skip the $

            char var_name[256];
            int j = 0;

            if (str[k] == '{')
            {
                k++; // skip the {
                while (str[k] != '\0' && str[k] != '}')
                    var_name[j++] = str[k++];

                if (str[k] == '\0')
                {
                    printf("Expansion Error: missing closing }\n");
                    return;
                }
            }
            else
            {
                while (str[k] != '\0' && (isalnum(str[k]) || str[k] == '_'))
                    var_name[j++] = str[k++];
                k--; // back up, the for loop will increment past this char
            }

            var_name[j] = '\0';

            char *val = Variables.get(var_name);
            if (val != NULL)
                strncat(result, val, sizeof(result) - strlen(result) - 1);
        }
        else
        {
            // append the literal character to result
            char ch[2] = {str[k], '\0'};
            strncat(result, ch, sizeof(result) - strlen(result) - 1);
        }
    }

    strncpy(arg->value, result, sizeof(arg->value) - 1);
    arg->value[sizeof(arg->value) - 1] = '\0';
}

void expand(Node *node)
{
    if (node == NULL)
        return;

    expand(node->left);
    expand(node->right);
    expand(node->body);

    if (node->type == CMD)
    {
        for (size_t i = 0; i < node->arg_count; i++)
        {
            if (node->args[i].needs_expansion && !node->args[i].is_literal)
                expand_arg(&node->args[i]);
        }
    }
}