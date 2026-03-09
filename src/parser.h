#ifndef PARSER_H
#define PARSER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "builtins.h"
#include "lexer.h"
#include "ast.h"

Node *parse_logical_expressions();
Node *parse_command_line();
Node *parse_pipeline();
Node *parse_command();
Node *parse_sub_command();
Node *parse(TokenList tokens_);

#endif
