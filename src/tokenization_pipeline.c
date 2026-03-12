#include "tokenization_pipeline.h"
#include "expander.h"
#include "executor.h"

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <windows.h>

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

    FILE *log_file = fopen(".log", "a");

    fprintf(log_file, "\n\nnew log starts here");

    // Print indentation
    for (int i = 0; i < indent; i++)
        fprintf(log_file, "  ");

    // Print node type
    switch (node->type)
    {
    case CMD_LINE:
        fprintf(log_file, "CMD_LINE\n");
        break;
    case PIPE:
        fprintf(log_file, "PIPE\n");
        break;
    case CMD:
        fprintf(log_file, "COMMAND\n");
        break;
    case SUB_COMMANDS:
        fprintf(log_file, "SUBSHELL\n");
        break;
    case AND:
        fprintf(log_file, "AND\n");
        break;
    case OR:
        fprintf(log_file, "OR\n");
        break;
    case ASSIGNMENT:
        fprintf(log_file, "ASSIGNMENT\n");
        break;
    default:
        fprintf(log_file, "UNKNOWN\n");
        break;
    }

    // Print args if this is a CMD node
    if (node->args)
    {
        for (int i = 0; i < node->arg_count; i++)
        {
            for (int j = 0; j < indent + 1; j++)
                fprintf(log_file, "  ");
            fprintf(log_file, "ARG: %s (type: %s)\n", get_Token_value(&node->args[i]), token_type_str(node->args[i].type));
        }
    }

    // Print redirects
    for (int i = 0; i < node->redirect_count; i++)
    {
        for (int j = 0; j < indent + 1; j++)
            fprintf(log_file, "   ");
        fprintf(log_file, "REDIRECT (%d) -> %s\n", node->redirects[i].type, node->redirects[i].filename);
    }

    // Print background flag
    if (node->background)
    {
        for (int j = 0; j < indent + 1; j++)
            fprintf(log_file, "  ");
        fprintf(log_file, "BACKGROUND: true\n");
    }

    // Recursively print children
    print_ast(node->left, indent + 1);
    print_ast(node->right, indent + 1);
    print_ast(node->body, indent + 1);

    fclose(log_file);
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