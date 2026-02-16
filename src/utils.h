#ifndef UTILS_H
#define UTILS_H

char *recursive_file_search(char fullpath[], char filename[]);

char *find_file(char filename[]);

char *get_var(const char *var);

void tokenize_cmd(char cmd[], char tokens[][50], int *no_of_tokens);

void set_pwd();

void set_home_directory();

void set_hostname();

void set_username();

void init_prompt();

#endif