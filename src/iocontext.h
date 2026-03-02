#ifndef IOCONTEXT_H
#define IOCONTEXT_H

#include <stdio.h>

typedef struct
{
    FILE *in;
    FILE *out;
    FILE *err;
} IOContext;

#endif
