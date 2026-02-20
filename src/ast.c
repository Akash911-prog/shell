#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ast.h"

static void _add_arg(struct Node *self, char *args)
{
    self->args = realloc(self->args, sizeof(char *) * (self->arg_count + 1));
    self->args[self->arg_count] = strdup(args);
    self->arg_count++;
}
static void _add_redirect(struct Node *self, Redirect redirect)
{
    self->redirects = realloc(self->redirects, sizeof(Redirect) * (self->redirect_count + 1));
    self->redirects[self->redirect_count] = redirect;
    self->redirect_count++;
}

Node *new_node(NodeType type, Node *left, Node *right)
{
    Node *node = malloc(sizeof(Node));
    if (!node)
    {
        perror("malloc failed");
        exit(1);
    }
    node->type = type;
    node->args = NULL;
    node->arg_count = 0;
    node->redirects = NULL;
    node->redirect_count = 0;
    node->left = left;
    node->right = right;
    node->body = NULL;
    node->background = false;
    node->add_arg = _add_arg;
    node->add_redirect = _add_redirect;

    return node;
}

void destroy_node(Node *node)
{
    if (!node)
        return;

    // Free args
    for (int i = 0; i < node->arg_count; i++)
    {
        free(node->args[i]); // free each string
    }
    free(node->args); // free the array

    // Free redirects
    for (int i = 0; i < node->redirect_count; i++)
    {
        free(node->redirects[i].filename); // free filename string
    }
    free(node->redirects); // free the array

    // Recursively destroy children
    destroy_node(node->left);
    destroy_node(node->right);
    destroy_node(node->body);

    // Finally, free the node itself
    free(node);
}
