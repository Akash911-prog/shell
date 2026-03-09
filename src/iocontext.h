#ifndef IOCONTEXT_H
#define IOCONTEXT_H

#include <stdio.h>

typedef struct
{
    FILE *in;
    FILE *out;
    FILE *err;
    bool close_in;
    bool close_out;
    bool close_err;
} IOContext;

typedef enum
{
    BUILT_IN,
    EXTERNAL
} Function_Type;

#endif
