#include "expression_evaluator.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>

static char *expression;

/* The different types of tokens we can encounter in an expression */
typedef enum
{
    TOKEN_NUMBER, /* a numeric value, e.g. 3.14 */
    TOKEN_PLUS,   /* + */
    TOKEN_MINUS,  /* - */
    TOKEN_STAR,   /* * */
    TOKEN_SLASH,  /* / */
    TOKEN_LPAREN, /* ( */
    TOKEN_RPAREN, /* ) */
    TOKEN_EOF     /* end of input */
} TokenType;

/* A token holds its type and, if it's a number, its numeric value */
typedef struct
{
    TokenType type;
    double value;
} Token;

static Token current_token; /* the token we're currently looking at */
static char *p;             /* our current position in the input string */

/*
 * Reads the next token from the input string and advances p past it.
 * Skips leading spaces before reading.
 */
static Token next_token()
{
    while (*p == ' ')
        p++;

    Token token;

    /* If the current character starts a number, read the whole number */
    if (isdigit(*p) || *p == '.')
    {
        char *end;
        token.type = TOKEN_NUMBER;
        token.value = strtod(p, &end); /* strtod reads as many digits as it can and sets end to where it stopped */
        p = end;
        return token;
    }

    if (*p == '+')
    {
        p++;
        token.type = TOKEN_PLUS;
        return token;
    }
    if (*p == '-')
    {
        p++;
        token.type = TOKEN_MINUS;
        return token;
    }
    if (*p == '*')
    {
        p++;
        token.type = TOKEN_STAR;
        return token;
    }
    if (*p == '/')
    {
        p++;
        token.type = TOKEN_SLASH;
        return token;
    }
    if (*p == '(')
    {
        p++;
        token.type = TOKEN_LPAREN;
        return token;
    }
    if (*p == ')')
    {
        p++;
        token.type = TOKEN_RPAREN;
        return token;
    }

    /* Nothing matched, so we've reached the end of the input */
    token.type = TOKEN_EOF;
    return token;
}

/*
 * If the current token matches the given token's type, advance to the next token.
 * Returns true if it matched, false otherwise.
 */
static bool consume(Token token)
{
    if (current_token.type == token.type)
    {
        current_token = next_token();
        return true;
    }
    return false;
}

/* Returns true if the current token is of the given type, without advancing */
static bool match(TokenType type)
{
    return current_token.type == type;
}

/*
 * Entry point. Takes the input string, sets up the parser state,
 * and kicks off evaluation.
 */
double eval(char *input, int *err)
{
    expression = input;
    p = expression;
    current_token = next_token(); /* load the first token */
    double value = eval_expression();
    if (!match(TOKEN_EOF))
        *err = 1; /* leftover tokens mean something went wrong */
    return value;
}

/*
 * Handles addition and subtraction.
 * Keeps evaluating terms and applying + or - until neither is found.
 *
 * expression → term (( "+" | "-" ) term)*
 */
double eval_expression()
{
    double left_value = eval_term();

    while (match(TOKEN_PLUS) || match(TOKEN_MINUS))
    {
        TokenType op = current_token.type;
        consume(current_token); /* move past the operator */
        double right_value = eval_term();

        if (op == TOKEN_PLUS)
            left_value = left_value + right_value;
        else if (op == TOKEN_MINUS)
            left_value = left_value - right_value;
    }

    return left_value;
}

/*
 * Handles multiplication and division.
 * Keeps evaluating factors and applying * or / until neither is found.
 * Called by eval_expression, so * and / bind tighter than + and -.
 *
 * term → factor (( "*" | "/" ) factor)*
 */
double eval_term()
{
    double left_value = eval_factor();

    while (match(TOKEN_STAR) || match(TOKEN_SLASH))
    {
        TokenType op = current_token.type;
        consume(current_token); /* move past the operator */
        double right_value = eval_factor();

        if (op == TOKEN_STAR)
            left_value = left_value * right_value;
        else if (op == TOKEN_SLASH)
            left_value = left_value / right_value;
    }

    return left_value;
}

/*
 * Handles the smallest unit of an expression: either a number or a
 * parenthesized sub-expression.
 *
 * factor → NUMBER | "(" expression ")"
 */
double eval_factor()
{
    if (match(TOKEN_LPAREN))
    {
        consume(current_token);           /* eat the '(' */
        double value = eval_expression(); /* evaluate what's inside */
        if (match(TOKEN_RPAREN))
            consume(current_token); /* eat the ')' */
        return value;
    }
    else if (match(TOKEN_NUMBER))
    {
        double val = current_token.value;
        current_token = next_token(); /* move past the number */
        return val;
    }
    else
    {
        /* We got something we didn't expect, e.g. a stray operator or unknown character */
        fprintf(stderr, "Unexpected token in factor\n");
        return 0.0;
    }
}