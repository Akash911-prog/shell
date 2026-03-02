#ifndef EXECUTOR_WIN_H
#define EXECUTOR_WIN_H

#include "ast.h"
#include "builtins.h"
#include "iocontext.h"
#include <stdio.h>

// default
IOContext default_io();

void execute_win(Node *node, IOContext io);

#endif