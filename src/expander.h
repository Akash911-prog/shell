#ifndef EXPANDER_H
#define EXPANDER_H

#include "ast.h"
#include "variables.h"

typedef enum
{
    SEG_LITERAL,
    SEG_TILDE,
    SEG_VARIABLE,
    SEG_VARIABLE_BRACED,
    SEG_EXPRESSION, // $((...))
    SEG_SUBSHELL,   // $(...)
    SEG_ASSIGN_NAME // name=rohan
} SegmentType;

typedef struct
{
    SegmentType type;
    const char *start;
    size_t len;
} Segment;

void expand(Node *node);

#endif