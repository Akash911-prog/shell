#ifndef COMMANDS_H
#define COMMANDS_H

#include "lexer.h"
#include "ast.h"
#include "executor_win.h"

int dummy(Node *node, IOContext io);

int echo(Node *node, IOContext io);

int type(Node *node, IOContext io);

int which(Node *node, IOContext io);

int clear(Node *node, IOContext io);

int pwd(Node *node, IOContext io);

int cd(Node *node, IOContext io);

int variable_handler(char var_name[]);

int run(char *filepath, Node *node);

#endif