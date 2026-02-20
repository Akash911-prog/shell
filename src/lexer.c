#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "lexer.h"

/**
 * lex() - Tokenizes a shell input string into a TokenList.
 *
 * Walks the input character by character and builds tokens based on
 * the following rules:
 *
 *  Character       Action
 *  -----------     -------------------------------------------------------
 *  '               Toggle is_literal. Not added to raw. (continue)
 *  "               Toggle is_quoted. Not added to raw. (continue)
 *  &&              Emit current word token + emit AND token. (continue)
 *  ||              Emit current word token + emit OR token. (continue)
 *  >>              Emit current word token + emit APPEND token. (continue)
 *  |               Emit current word token + emit PIPE token. (continue)
 *  >               Emit current word token + emit REDIRECT_OUT token. (continue)
 *  <               Emit current word token + emit REDIRECT_IN token. (continue)
 *  \               Advance i to skip backslash, fall through to add next char.
 *  $               Set needs_expansion = true on current token, fall through.
 *  ~               Set needs_expansion = true on current token, fall through.
 *  anything else   Fall through, added to raw[].
 *  (is_literal)    Skips the if(!is_literal) block entirely, added to raw[].
 *
 * @param data          The raw input string to tokenize.
 * @param token_list    Pointer to TokenList to populate.
 */
void lex(char *data, TokenList *token_list)
{
    int i = 0; // index of data
    int j = 0; // index of token
    int k = 0; // index of char inside token

    bool is_literal = false;
    bool is_quoted = false;

    while (data[i] != '\0')
    {

        // Whitespace - end current word token
        if (data[i] == ' ' && !is_literal && !is_quoted)
        {
            if (k > 0)
            {
                token_list->tokens[j].raw[k] = '\0';
                token_list->tokens[j].type = TOKEN_WORD;
                token_list->tokens[j].is_literal = is_literal;
                token_list->tokens[j].is_quoted = is_quoted;
                token_list->tokens[j].position = j;
                j++;
                k = 0;
            }
            i++;
            continue;
        }

        // Single quote toggle
        if (data[i] == '\'')
        {
            is_literal = !is_literal;

            if (is_literal)
            {
                token_list->tokens[j].is_literal = true;
            }
            else
            {
                // Just turned OFF - end the literal token now
                if (k > 0)
                {
                    token_list->tokens[j].raw[k] = '\0';
                    token_list->tokens[j].position = j;
                    j++;
                    k = 0;
                    is_literal = false;
                }
            }

            i++;
            continue;
        }

        if (!is_literal)
        {
            // Double quote toggle
            if (data[i] == '"')
            {
                is_quoted = !is_quoted;

                if (is_quoted)
                {
                    token_list->tokens[j].is_quoted = true;
                }
                else
                {
                    // Just turned OFF - end the quoted token now
                    if (k > 0)
                    {
                        token_list->tokens[j].raw[k] = '\0';
                        token_list->tokens[j].position = j;
                        j++;
                        k = 0;
                        is_quoted = false;
                    }
                }
                i++;
                continue;
            }

            // AND &&
            else if (data[i] == '&' && data[i + 1] == '&')
            {
                if (k > 0)
                {
                    token_list->tokens[j].raw[k] = '\0';
                    token_list->tokens[j].type = TOKEN_WORD;
                    token_list->tokens[j].position = j;
                    j++;
                    k = 0;
                }
                token_list->tokens[j].type = TOKEN_AND;
                strcpy(token_list->tokens[j].raw, "&&");
                token_list->tokens[j].position = j;
                j++;
                i += 2;
                continue;
            }

            // OR ||
            else if (data[i] == '|' && data[i + 1] == '|')
            {
                if (k > 0)
                {
                    token_list->tokens[j].raw[k] = '\0';
                    token_list->tokens[j].type = TOKEN_WORD;
                    token_list->tokens[j].position = j;
                    j++;
                    k = 0;
                }
                token_list->tokens[j].type = TOKEN_OR;
                strcpy(token_list->tokens[j].raw, "||");
                token_list->tokens[j].position = j;
                j++;
                i += 2;
                continue;
            }

            // APPEND >>
            else if (data[i] == '>' && data[i + 1] == '>')
            {
                if (k > 0)
                {
                    token_list->tokens[j].raw[k] = '\0';
                    token_list->tokens[j].type = TOKEN_WORD;
                    token_list->tokens[j].position = j;
                    j++;
                    k = 0;
                }
                token_list->tokens[j].type = TOKEN_REDIRECT_APPEND;
                strcpy(token_list->tokens[j].raw, ">>");
                token_list->tokens[j].position = j;
                j++;
                i += 2;
                continue;
            }

            // PIPE |
            else if (data[i] == '|')
            {
                if (k > 0)
                {
                    token_list->tokens[j].raw[k] = '\0';
                    token_list->tokens[j].type = TOKEN_WORD;
                    token_list->tokens[j].position = j;

                    j++;
                    k = 0;
                }
                token_list->tokens[j].type = TOKEN_PIPE;
                strcpy(token_list->tokens[j].raw, "|");
                token_list->tokens[j].position = j;
                j++;
                i++;
                continue;
            }

            // REDIRECT_OUT >
            else if (data[i] == '>')
            {
                if (k > 0)
                {
                    token_list->tokens[j].raw[k] = '\0';
                    token_list->tokens[j].type = TOKEN_WORD;
                    token_list->tokens[j].position = j;

                    j++;
                    k = 0;
                }
                token_list->tokens[j].type = TOKEN_REDIRECT_OUT;
                strcpy(token_list->tokens[j].raw, ">");
                token_list->tokens[j].position = j;
                token_list->tokens[j].position = j;
                j++;
                i++;
                continue;
            }

            // REDIRECT_IN
            else if (data[i] == '<')
            {
                if (k > 0)
                {
                    token_list->tokens[j].raw[k] = '\0';
                    token_list->tokens[j].type = TOKEN_WORD;
                    token_list->tokens[j].position = j;
                    j++;
                    k = 0;
                }
                token_list->tokens[j].type = TOKEN_REDIRECT_IN;
                strcpy(token_list->tokens[j].raw, "<");
                token_list->tokens[j].position = j;
                j++;
                i++;
                continue;
            }

            // Escape sequence - skip backslash, fall through to add next char
            else if (data[i] == '\\' && data[i + 1] != '\0')
            {
                i++; // skip the backslash, add next char at bottom
            }

            // Variable/expansion
            else if (data[i] == '$')
            {
                token_list->tokens[j].needs_expansion = true;
                // fall through to add $ to raw
            }

            // Tilde expansion
            else if (data[i] == '~')
            {
                token_list->tokens[j].needs_expansion = true;
                // fall through to add ~ to raw
            }
        }

        // BOTTOM: all word chars land here (regular, $, ~, escaped, literal)
        token_list->tokens[j].raw[k++] = data[i];
        i++;
    }

    // Last token
    if (k > 0)
    {
        token_list->tokens[j].raw[k] = '\0';
        token_list->tokens[j].type = TOKEN_WORD;
        token_list->tokens[j].is_literal = is_literal;
        token_list->tokens[j].is_quoted = is_quoted;
        token_list->tokens[j].position = j;
        j++;
    }

    token_list->count = j;
}
