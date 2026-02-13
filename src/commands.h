#ifndef COMMANDS_H
#define COMMANDS_H

void echo(char tokens[][50], int no_of_tokens);

int type(char tokens[][50]);

char *get_var(const char *var);

char *find_file(char filename[]);

#endif