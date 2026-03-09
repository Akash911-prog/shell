#include "tokenization_pipeline.h"
#include "expander.h"
#include "executor.h"

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>

static const char *token_type_str(Type type)
{
    switch (type)
    {
    case TOKEN_WORD:
        return "WORD";
    case TOKEN_ASSIGN:
        return "ASSIGN";
    case TOKEN_PIPE:
        return "PIPE";
    case TOKEN_AND:
        return "AND";
    case TOKEN_OR:
        return "OR";
    case TOKEN_REDIRECT_IN:
        return "REDIRECT_IN";
    case TOKEN_REDIRECT_OUT:
        return "REDIRECT_OUT";
    case TOKEN_REDIRECT_APPEND:
        return "REDIRECT_APPEND";
    case TOKEN_REDIRECT_ERR:
        return "REDIRECT_ERR";
    case TOKEN_REDIRECT_ERR_APPEND:
        return "REDIRECT_ERR_APPEND";
    case TOKEN_BACKGROUND:
        return "BACKGROUND";
    case TOKEN_SEMICOLON:
        return "SEMICOLON";
    case TOKEN_LPAREN:
        return "LPAREN";
    case TOKEN_RPAREN:
        return "RPAREN";
    case TOKEN_EOF:
        return "EOF";
    default:
        return "UNKNOWN";
    }
}

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
    case ASSIGNMENT:
        printf("ASSIGNMENT\n");
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
            printf("ARG: %s (type: %s)\n", get_Token_value(&node->args[i]), token_type_str(node->args[i].type));
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

void tokenize(char *input, TokenList *tokens)
{
    tokens->tokens = malloc(sizeof(Token) * 256);
    tokens->count = 0;
    lex(input, tokens);
}

void destroy_tokens(TokenList *tokens)
{
    if (tokens == NULL)
        return;
    free(tokens->tokens);
    tokens->tokens = NULL;
    tokens->count = 0;
}

void exec_input(char *input)
{
    TokenList tokens = {NULL, 0};
    tokenize(input, &tokens);
    Node *tree = parse(tokens);
    destroy_tokens(&tokens);
    if (tree == NULL)
    {
        return;
    }
    expand(tree);
    print_ast(tree, 2);
    execute(tree);
    destroy_node(tree);
}