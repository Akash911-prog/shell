#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ast.h"

void add_arg(struct Node *self, char *args)
{
    self->args = realloc(self->args, sizeof(char *) * (self->arg_count + 1));
    self->args[self->arg_count] = strdup(args);
    self->arg_count++;
}
void add_redirect(struct Node *self, Redirect redirect)
{
    self->redirects = realloc(self->redirects, sizeof(Redirect) * (self->redirect_count + 1));
    self->redirects[self->redirect_count] = redirect;
    self->redirect_count++;
}

Node *new_node(NodeType type)
{
    Node *node = malloc(sizeof(Node));
    node->type = type;
    node->args = NULL;
    node->arg_count = 0;
    node->redirects = NULL;
    node->redirect_count = 0;
    node->left = NULL;
    node->right = NULL;
    node->body = NULL;
    node->background = false;
    node->add_arg = add_arg;
    node->add_redirect = add_redirect;

    return node;
}

void destroy_tree(Node *node)
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
