#ifndef TOKENS_H
#define TOKENS_H

#include <stdbool.h>

typedef enum
{
    TOKEN_WORD,
    TOKEN_PIPE,
    TOKEN_REDIRECT_IN,
    TOKEN_REDIRECT_OUT,
    TOKEN_REDIRECT_APPEND,
    TOKEN_BACKGROUND,
    TOKEN_AND,
    TOKEN_OR,
    TOKEN_SEMICOLON,
} TokenType;

typedef struct
{
    char raw[256];   // Original text: "$USER/documents"
    char value[256]; // Expanded text: "/home/john/documents"
    TokenType type;
    bool is_quoted;       // Double quoted - expand vars but not globs
    bool is_literal;      // Single quoted - expand nothing
    bool needs_expansion; // Contains $VAR or * or ~
    int position;
} Token;

typedef struct
{
    Token tokens[50];
    int count;
} TokenList;

void lex(char *data, TokenList *token_list);

#endif