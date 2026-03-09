#ifndef RUN_H
#define RUN_H

// for both unix and windows
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "iocontext.h"
#include "ast.h"
#include "variables.h"

#ifdef _WIN32 // for windows
#include <windows.h>
#include <process.h>
#include <io.h>

#else // for unix
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#endif

int run(char *filepath, Node *node, IOContext *io);

#endif