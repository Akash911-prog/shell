#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "lexer.h"
#include "parser.h"

Token *current_token = NULL;

Token *get_next_token()
{
    size_t next = current_token->position + 1;
    if (next >= tokens.count)
    {
        return current_token; // stay at EOF, don't go out of bounds
    }
    return &tokens.tokens[next];
}

bool match(TokenType type)
{
    if (current_token->type == type)
    {
        return true;
    }
    return false;
}

bool consume(TokenType type)
{
    if (current_token->type == type)
    {
        current_token = get_next_token();
        return true;
    }
    printf("Parsing Error on token %d\n", current_token->position);
    return false;
}

bool is_redirect(TokenType type)
{
    if (type == TOKEN_REDIRECT_APPEND ||
        type == TOKEN_REDIRECT_ERR ||
        type == TOKEN_REDIRECT_ERR_APPEND ||
        type == TOKEN_REDIRECT_IN ||
        type == TOKEN_REDIRECT_OUT)
    {
        return true;
    }
    return false;
}

void attach_redirect(Node *cmd)
{
    int fd;
    if (match(TOKEN_REDIRECT_ERR) || match(TOKEN_REDIRECT_ERR_APPEND))
    {
        fd = 2;
    }
    else if (match(TOKEN_REDIRECT_OUT) || match(TOKEN_REDIRECT_APPEND))
    {
        fd = 1;
    }
    else
    {
        fd = 0;
    }

    Redirect new_redirect = {.filename = NULL, .type = current_token->type, .fd = fd};
    current_token = get_next_token();

    if (match(TOKEN_WORD))
    {
        new_redirect.filename = strdup(current_token->raw);
        current_token = get_next_token();
    }

    cmd->add_redirect(cmd, new_redirect);
}

Node *parse_logical_expressions()
{
    Node *left = parse_command_line();
    while (match(TOKEN_AND) || match(TOKEN_OR))
    {
        TokenType op = current_token->type;
        consume(current_token->type);
        Node *right = parse_command_line();
        if (op == TOKEN_AND)
        {
            left = new_node(AND, left, right);
        }
        else
        {
            left = new_node(OR, left, right);
        }
    }
    return left;
}

Node *parse_command_line()
{
    Node *left = parse_pipeline();
    while (match(TOKEN_SEMICOLON))
    {
        consume(current_token->type);
        Node *right = parse_pipeline();
        left = new_node(CMD_LINE, left, right);
    }
    return left;
}

Node *parse_pipeline()
{
    Node *left = parse_command();
    while (match(TOKEN_PIPE))
    {
        consume(current_token->type);
        Node *right = parse_command();
        left = new_node(PIPE, left, right);
    }
    return left;
}

Node *parse_command()
{
    Node *cmd = parse_sub_command();
    while (is_redirect(current_token->type))
    {
        attach_redirect(cmd);
    }
    // Fix #4: TOKEN_BACKGROUND was matched but never consumed, leaving the
    // parser stuck on the same token.
    if (match(TOKEN_BACKGROUND))
    {
        cmd->background = true;
        consume(TOKEN_BACKGROUND);
    }
    return cmd;
}

Node *parse_sub_command()
{
    if (match(TOKEN_LPAREN))
    {
        consume(TOKEN_LPAREN);
        Node *body = parse_logical_expressions();
        consume(TOKEN_RPAREN);
        Node *node = new_node(SUB_COMMANDS, NULL, NULL);
        node->body = body;
        return node;
    }
    else
    {
        Node *node = new_node(CMD, NULL, NULL);
        if (match(TOKEN_ASSIGN))
        {
            Node *node = new_node(ASSIGNMENT, NULL, NULL);
            while (match(TOKEN_ASSIGN))
            {
                node->add_arg(node, current_token->raw);
                consume(TOKEN_ASSIGN);
            }
            return node;
        }
        while (match(TOKEN_WORD))
        {
            node->add_arg(node, current_token);
            consume(TOKEN_WORD);
        }
        return node;
    }
}