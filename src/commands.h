#ifndef COMMANDS_H
#define COMMANDS_H

#include "lexer.h"

int dummy(TokenList *token_list);

int echo(TokenList *token_list);

int type(TokenList *token_list);

int which(TokenList *token_list);

int clear(TokenList *token_list);

int pwd(TokenList *token_list);

int cd(TokenList *token_list);

int variable_handler(char var_name[]);

int execute(char *filepath, TokenList *token_list);

#endif