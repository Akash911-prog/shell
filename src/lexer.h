#ifndef TOKENS_H
#define TOKENS_H

#include <stdbool.h>

/*
 LOWEST PRECEDENCE (binds loosest, evaluated last)
 1. TOKEN_BACKGROUND           // &     (run in background)
 2. TOKEN_AND                  // &&    (conditional execution)
     TOKEN_OR                  // ||    (conditional execution)
 3. TOKEN_PIPE                 // |     (pipeline)
 4. TOKEN_REDIRECT_IN          // <     (redirections)
    TOKEN_REDIRECT_OUT         // >
    TOKEN_REDIRECT_APPEND      // >>
 5. TOKEN_WORD                 // arguments, commands
 HIGHEST PRECEDENCE (binds tightest, evaluated first) */
typedef enum
{
    TOKEN_WORD,
    TOKEN_PIPE,
    TOKEN_REDIRECT_IN,
    TOKEN_REDIRECT_OUT,
    TOKEN_REDIRECT_APPEND,
    TOKEN_REDIRECT_ERR,        // Add
    TOKEN_REDIRECT_ERR_APPEND, // Add
    TOKEN_BACKGROUND,          // Add
    TOKEN_SEMICOLON,           // Add
    TOKEN_LPAREN,              // Add
    TOKEN_RPAREN,              // Add
    TOKEN_AND,
    TOKEN_OR,
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