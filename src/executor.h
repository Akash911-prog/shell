#ifndef EXECUTOR_H
#define EXECUTOR_H

#if _WIN32
#include "executor_win.h"
#else
#include "executor_unix.h"
#endif

void execute(Node *node);

#endif