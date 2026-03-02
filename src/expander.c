#include "expander.h"
#include <string.h>
#include <ctype.h>
#include <stdio.h>
#include "expression_evaluator.h"

#define RESULT_CAP 4096

static void append(char *result, size_t *r, const char *src, size_t len)
{
    size_t space = RESULT_CAP - *r - 1;
    size_t n = len < space ? len : space;
    memcpy(result + *r, src, n);
    *r += n;
}

static Segment next_segment(const char **p)
{
    Segment seg = {0};

    if (**p == '\0')
        return seg;

    /* check for assignment: a valid variable name followed by = */
    const char *lookahead = *p;
    while (isalnum((unsigned char)*lookahead) || *lookahead == '_')
        lookahead++;

    if (lookahead > *p && *lookahead == '=')
    {
        seg.type = SEG_ASSIGN_NAME;
        seg.start = *p;
        seg.len = lookahead - *p; /* just the name, without the = */
        *p = lookahead + 1;       /* skip past the = */
        return seg;
    }

    if (**p != '$')
    {
        seg.type = SEG_LITERAL;
        seg.start = *p;
        while (**p && **p != '$')
            (*p)++;
        seg.len = *p - seg.start;
        return seg;
    }

    (*p)++; // skip $

    if (**p == '(' && *(*p + 1) == '(')
    {
        // $((expression))
        (*p) += 2;
        seg.type = SEG_EXPRESSION;
        seg.start = *p;
        while (**p && !(**p == ')' && *(*p + 1) == ')'))
            (*p)++;
        seg.len = *p - seg.start;
        if (**p)
            (*p) += 2; // skip ))
    }
    else if (**p == '(')
    {
        // $(subshell)
        (*p)++;
        seg.type = SEG_VARIABLE_BRACED;
        seg.start = *p;
        while (**p && **p != ')')
            (*p)++;
        seg.len = *p - seg.start;
        if (**p)
            (*p)++; // skip )
    }
    else if (**p == '{')
    {
        (*p)++;
        seg.type = SEG_VARIABLE_BRACED;
        seg.start = *p;
        while (**p && **p != '}')
            (*p)++;
        seg.len = *p - seg.start;
        if (**p)
            (*p)++; // skip }
    }
    else
    {
        seg.type = SEG_VARIABLE;
        seg.start = *p;
        while (isalnum((unsigned char)**p) || **p == '_')
            (*p)++;
        seg.len = *p - seg.start;
    }

    return seg;
}

static void expand_variable(const Segment *seg, char *result, size_t *r)
{
    char var_name[256];
    size_t len = seg->len < 255 ? seg->len : 255;
    memcpy(var_name, seg->start, len);
    var_name[len] = '\0';

    const char *val = Variables.get(var_name);
    if (val)
        append(result, r, val, strlen(val));
    else
    {
        char buff[256];
        snprintf(buff, sizeof(buff), "%s: variable not found", var_name);
        append(result, r, buff, strlen(buff));
    }
}

static void expand_expression(const Segment *seg, char *result, size_t *r)
{
    char expression[256];
    size_t len = seg->len < 255 ? seg->len : 255;
    memcpy(expression, seg->start, len);
    expression[len] = '\0'; // don't forget to null terminate

    int err = 0;
    double value = eval(expression, &err);

    if (err != 0)
    {
        printf("Expansion Error: invalid expression\n");
        return;
    }

    char buffer[64];
    snprintf(buffer, sizeof(buffer), "%.17g", value);
    append(result, r, buffer, strlen(buffer));
}

static void expand_assign(Node *node, int arg_index)
{
    Token *token = &node->args[arg_index];
    char *p = token->raw;

    char name[256] = {0};
    char value[256] = {0};
    int ni = 0, vi = 0;
    bool found = false;

    while (*p != '\0')
    {
        if (!found && *p == '=')
        {
            found = true;
            p++;
            continue;
        }
        if (found)
            value[vi++] = *p;
        else
            name[ni++] = *p;
        p++;
    }

    name[ni] = '\0';
    value[vi] = '\0';

    /* store back - expand the value side through normal expansion */
    strncpy(node->args[arg_index].raw, name, sizeof(token->raw) - 1);
    strncpy(node->args[arg_index].value, value, sizeof(token->value) - 1);
}

static void expand_arg(Token *arg)
{
    char result[RESULT_CAP];
    size_t r = 0;
    const char *p = arg->raw;

    // tilde still handled upfront
    if (p[0] == '~' && (p[1] == '/' || p[1] == '\0'))
    {
        const char *home = Variables.get("HOME");
        append(result, &r, home ? home : "~", home ? strlen(home) : 1);
        p++;
    }

    while (*p)
    {
        Segment seg = next_segment(&p);
        switch (seg.type)
        {
        case SEG_LITERAL:
            append(result, &r, seg.start, seg.len);
            break;
        case SEG_VARIABLE:
        case SEG_VARIABLE_BRACED:
            expand_variable(&seg, result, &r);
            break;
        case SEG_EXPRESSION:
            expand_expression(&seg, result, &r);
            break;
        }
    }

    result[r] = '\0';

#ifdef _WIN32
    for (size_t i = 0; i < r; i++)
        if (result[i] == '/')
            result[i] = '\\';
#endif

    strncpy(arg->value, result, sizeof(arg->value) - 1);
    arg->value[sizeof(arg->value) - 1] = '\0';
}

void expand(Node *node)
{
    if (node == NULL)
        return;

    expand(node->left);
    expand(node->right);
    expand(node->body);

    if (node->type == CMD || node->type == ASSIGNMENT)
        for (size_t i = 0; i < node->arg_count; i++)
        {
            if (node->args[i].needs_expansion && !node->args[i].is_literal)
            {
                expand_arg(&node->args[i]);
            }
            if (node->args[i].type == TOKEN_ASSIGN)
            {
                expand_assign(node, i);
            }
        }
}