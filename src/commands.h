#ifndef COMMANDS_H
#define COMMANDS_H

int dummy(char tokens[][50], int no_of_tokens);

int echo(char tokens[][50], int no_of_tokens);

int type(char tokens[][50], int no_of_tokens);

int which(char tokens[][50], int no_of_tokens);

int clear(char tokens[][50], int no_of_tokens);

int variable_handler(char var_name[]);

int execute(char *filepath, char tokens[][50], int no_of_tokens);

#endif