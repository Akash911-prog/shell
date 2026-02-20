#ifndef PARSER_H
#define PARSER_H

#include "ast.h"

Node *parse_logical_expressions();
Node *parse_command_line();
Node *parse_pipeline();
Node *parse_command();
Node *parse_sub_command();

extern Token *current_token;

#endif
