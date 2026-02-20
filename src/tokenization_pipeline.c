#include "tokenization_pipeline.h"

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>

void print_ast(Node *node, int indent)
{
    if (!node)
        return;

    // Print indentation
    for (int i = 0; i < indent; i++)
        printf("  ");

    // Print node type
    switch (node->type)
    {
    case CMD_LINE:
        printf("CMD_LINE\n");
        break;
    case PIPE:
        printf("PIPE\n");
        break;
    case CMD:
        printf("COMMAND\n");
        break;
    case SUB_COMMANDS:
        printf("SUBSHELL\n");
        break;
    case AND:
        printf("AND\n");
        break;
    case OR:
        printf("OR\n");
        break;
    default:
        printf("UNKNOWN\n");
        break;
    }

    // Print args if this is a CMD node
    if (node->args)
    {
        for (int i = 0; i < node->arg_count; i++)
        {
            for (int j = 0; j < indent + 1; j++)
                printf("  ");
            printf("ARG: %s\n", node->args[i]);
        }
    }

    // Print redirects
    for (int i = 0; i < node->redirect_count; i++)
    {
        for (int j = 0; j < indent + 1; j++)
            printf("  ");
        printf("REDIRECT (%d) -> %s\n", node->redirects[i].type, node->redirects[i].filename);
    }

    // Print background flag
    if (node->background)
    {
        for (int j = 0; j < indent + 1; j++)
            printf("  ");
        printf("BACKGROUND: true\n");
    }

    // Recursively print children
    print_ast(node->left, indent + 1);
    print_ast(node->right, indent + 1);
    print_ast(node->body, indent + 1);
}

void tokenize(char *input)
{
    lex(input, &tokens);
    current_token = &tokens.tokens[0];
}

Node *parse()
{
    return parse_logical_expressions();
}

void parsecute(char *input)
{
    tokenize(input);
    Node *tree = parse();
    if (tree == NULL)
    {
        return;
    }
    print_ast(tree, 4);
    destroy_node(tree);
}